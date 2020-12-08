using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static class Sen1PatchRun {
		private static HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer TryLoadBackupArchive(string backupArchivePath) {
			try {
				if (File.Exists(backupArchivePath)) {
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(backupArchivePath)) {
						return new HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer(fs);
					}
				}
			} catch (Exception ex) {
				Console.WriteLine("Failed to load backup file archive: {0}", ex.Message);
			}
			return null;
		}

		public static bool Run(string baseDir) {
			if (!Directory.Exists(baseDir)) {
				Console.WriteLine("No directory found at {0}.", baseDir);
				return false;
			}

			string backupArchivePath = Path.Combine(baseDir, "senpatcher_rerun_revert_data.bin");
			FileStorage.InitReturnValue fileStoreReturnValue = null;
			using (HyoutaUtils.HyoutaArchive.HyoutaArchiveContainer backupArchive = TryLoadBackupArchive(backupArchivePath)) {
				fileStoreReturnValue = FileStorage.InitializeFromKnownFiles(baseDir, Sen1KnownFiles.Files, backupArchive);
			}

			if (fileStoreReturnValue.Errors.Count == 0) {
				if (fileStoreReturnValue.NewFileFound) {
					Stream ms = new MemoryStream();
					fileStoreReturnValue.Storage.WriteToHyoutaArchive(ms);
					using (var fs = new FileStream(backupArchivePath, FileMode.Create)) {
						ms.Position = 0;
						HyoutaUtils.StreamUtils.CopyStream(ms, fs);
					}
				}
			}

			return true;
		}
	}
}
