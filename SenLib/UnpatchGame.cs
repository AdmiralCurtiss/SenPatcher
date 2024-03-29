using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Checksum;
using HyoutaUtils.HyoutaArchive;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class UnpatchGame {
		public static bool HasOldSenpatcherBackups(string gamepath, int sengame) {
			return File.Exists(Path.Combine(gamepath, "senpatcher_rerun_revert_data.bin"))
				|| Directory.Exists(Path.Combine(gamepath, "senpatcher_bkp"))
				|| (sengame == 1 && File.Exists(Path.Combine(gamepath, "ed8.exe.senpatcher.bkp")))
				|| (sengame == 1 && File.Exists(Path.Combine(gamepath, "ed8jp.exe.senpatcher.bkp")))
				|| (sengame == 2 && File.Exists(Path.Combine(gamepath, "bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp")))
				|| (sengame == 2 && File.Exists(Path.Combine(gamepath, "bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp")));
		}

		private static void CheckFile(List<FileWithHashAndMaybePath> existingFiles, UnpatchFile[] unpatchFiles, string path) {
			try {
				using (var stream = new DuplicatableFileStream(path)) {
					// 10 MB limit, none of our backup files should be this big
					if (stream.Length > 10000000) {
						return;
					}
					var bytestream = stream.CopyToByteArrayStreamAndDispose();
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(bytestream);
					if (unpatchFiles.Any(x => x.Hash == hash || (x.AltHash.HasValue && x.AltHash.Value == hash))) {
						existingFiles.Add(new FileWithHashAndMaybePath() { Data = bytestream, Hash = hash, Path = path });
					}
				}
			} catch (Exception ex) {
				Logging.Log("Failed to open file " + path + ":" + ex.Message);
			}
		}

		public static bool Unpatch(string gamepath, UnpatchFile[] unpatchFiles, int sengame) {
			List<FileWithHashAndMaybePath> existingFiles = new List<FileWithHashAndMaybePath>();
			if (Directory.Exists(Path.Combine(gamepath, "senpatcher_bkp"))) {
				foreach (string path in Directory.GetFiles(Path.Combine(gamepath, "senpatcher_bkp"), "*", SearchOption.AllDirectories)) {
					CheckFile(existingFiles, unpatchFiles, path);
				}
			}
			if (sengame == 1 && File.Exists(Path.Combine(gamepath, "ed8.exe.senpatcher.bkp"))) {
				CheckFile(existingFiles, unpatchFiles, Path.Combine(gamepath, "ed8.exe.senpatcher.bkp"));
			}
			if (sengame == 1 && File.Exists(Path.Combine(gamepath, "ed8jp.exe.senpatcher.bkp"))) {
				CheckFile(existingFiles, unpatchFiles, Path.Combine(gamepath, "ed8jp.exe.senpatcher.bkp"));
			}
			if (sengame == 2 && File.Exists(Path.Combine(gamepath, "bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"))) {
				CheckFile(existingFiles, unpatchFiles, Path.Combine(gamepath, "bin/Win32/ed8_2_PC_US.exe.senpatcher.bkp"));
			}
			if (sengame == 2 && File.Exists(Path.Combine(gamepath, "bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"))) {
				CheckFile(existingFiles, unpatchFiles, Path.Combine(gamepath, "bin/Win32/ed8_2_PC_JP.exe.senpatcher.bkp"));
			}
			if (File.Exists(Path.Combine(gamepath, "senpatcher_rerun_revert_data.bin"))) {
				try {
					using (var stream = new DuplicatableFileStream(Path.Combine(gamepath, "senpatcher_rerun_revert_data.bin"))) {
						var hac = new HyoutaArchiveContainer(stream);
						for (long i = 0; i < hac.Filecount; ++i) {
							using (var file = hac.GetChildByIndex(i).AsFile)
							using (var filestream = file.DataStream.Duplicate()) {
								var hash = ChecksumUtils.CalculateSHA1ForEntireStream(filestream);
								if (unpatchFiles.Any(x => x.Hash == hash || (x.AltHash.HasValue && x.AltHash.Value == hash))) {
									existingFiles.Add(new FileWithHashAndMaybePath() { Data = filestream.CopyToByteArrayStreamAndDispose(), Hash = hash, Path = null });
								}
							}
						}
					}
				} catch (Exception ex) {
					Logging.Log("Error in rerun/revert data:" + ex.Message);
				}
			}

			bool unpatchedAll = true;
			foreach (UnpatchFile unpatchFile in unpatchFiles) {
				using (var filestream = new DuplicatableFileStream(Path.Combine(gamepath, unpatchFile.Path))) {
					var hash = ChecksumUtils.CalculateSHA1ForEntireStream(filestream);
					if (unpatchFile.Hash == hash || (unpatchFile.AltHash.HasValue && unpatchFile.AltHash.Value == hash)) {
						// this is already unpatched, nothing to do
						continue;
					}
				}

				// if we reach here we need to unpatch the file, see if we have a copy in our files
				var existingFile = existingFiles.Where(x => x.Hash == unpatchFile.Hash).FirstOrDefault();
				if (existingFile == null && unpatchFile.AltHash.HasValue) {
					existingFile = existingFiles.Where(x => x.Hash == unpatchFile.AltHash.Value).FirstOrDefault();
				}
				if (existingFile == null) {
					// we don't, we can't unpatch all files
					unpatchedAll = false;
					continue;
				}

				using (var fs = new FileStream(Path.Combine(gamepath, unpatchFile.Path), FileMode.Create)) {
					existingFile.Data.Position = 0;
					StreamUtils.CopyStream(existingFile.Data, fs);
				}
			}

			if (unpatchedAll) {
				// all files are unpatched, we can remove the backup files
				foreach (var existingFile in existingFiles) {
					if (existingFile.Path != null) {
						SenUtils.TryDeleteFile(existingFile.Path);
					}
				}
				SenUtils.TryDeleteFile(Path.Combine(gamepath, "senpatcher_rerun_revert_data.bin"));
				SenUtils.TryDeleteEmptyDirectory(Path.Combine(gamepath, "senpatcher_bkp"));
			}

			return unpatchedAll;
		}
	}

	public class FileWithHashAndMaybePath {
		public DuplicatableStream Data;
		public SHA1 Hash;
		public string Path;
	}
}
