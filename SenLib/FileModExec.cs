using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib {
	public class FileModExec {
		private static HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer TryLoadBackupArchive(string backupArchivePath) {
			try {
				if (File.Exists(backupArchivePath)) {
					Console.WriteLine("Loading backup archive at {0}...", backupArchivePath);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(backupArchivePath)) {
						return new HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer(fs);
					}
				}
			} catch (Exception ex) {
				Console.WriteLine("Failed to load backup file archive: {0}", ex.Message);
			}
			return null;
		}

		public static FileStorage InitializeAndPersistFileStorage(string baseDir, KnownFile[] knownFiles) {
			if (!Directory.Exists(baseDir)) {
				Console.WriteLine("No directory found at {0}.", baseDir);
				return null;
			}

			string backupArchivePath = Path.Combine(baseDir, "senpatcher_rerun_revert_data.bin");
			FileStorage.InitReturnValue fileStoreReturnValue = null;
			using (HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer backupArchive = TryLoadBackupArchive(backupArchivePath)) {
				fileStoreReturnValue = FileStorage.InitializeFromKnownFiles(baseDir, knownFiles, backupArchive);
			}

			if (fileStoreReturnValue.ShouldWriteBackupArchive) {
				Console.WriteLine("New file(s) found, writing new backup archive to {0}...", backupArchivePath);
				Stream ms = new MemoryStream();
				fileStoreReturnValue.Storage.WriteToHyoutaArchive(ms);
				using (var fs = new FileStream(backupArchivePath, FileMode.Create)) {
					ms.Position = 0;
					HyoutaUtils.StreamUtils.CopyStream(ms, fs);
				}
			}

			return fileStoreReturnValue.Storage;
		}

		public static PatchResult ExecuteMods(string gamedir, FileStorage storage, IEnumerable<FileMod> mods) {
			int total = 0;
			int success = 0;

			foreach (FileMod mod in mods) {
				Console.WriteLine("Executing mod: {0}", mod.GetDescription());

				++total;

				var results = mod.TryApply(storage);
				if (results == null) {
					Console.WriteLine("Failed to apply: {0}", mod.GetDescription());
					continue;
				}

				bool diskWriteSuccess = true;
				foreach (FileModResult result in results) {
					string path = Path.Combine(gamedir, result.TargetPath);
					if (result.ResultData != null) {
						if (!SenUtils.TryWriteFileIfDifferent(result.ResultData, path)) {
							Console.WriteLine("Failed to write to disk: {0}", path);
							diskWriteSuccess = false;
						}
					} else {
						if (!SenUtils.TryDeleteFile(path)) {
							Console.WriteLine("Failed to write delete from disk: {0}", path);
							diskWriteSuccess = false;
						}
					}
				}

				if (diskWriteSuccess) {
					++success;
				}
			}

			return new PatchResult(total, success);
		}

		public static PatchResult RevertMods(string gamedir, FileStorage storage, IEnumerable<FileMod> mods) {
			return ExecuteMods(gamedir, storage, InvertMods(mods));
		}

		public static IEnumerable<FileMod> InvertMods(IEnumerable<FileMod> mods) {
			foreach (FileMod mod in mods) {
				yield return new FileModInvert(mod);
			}
		}
	}
}
