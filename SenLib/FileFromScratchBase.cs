using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public abstract class FileFromScratchBase : FileFix {
		public bool TryApply(string basepath, string backuppath) {
			string subtargetpath = GetSubTargetPath();
			string sha1 = GetTargetSha1();
			string targetpath = Path.Combine(basepath, subtargetpath);

			if (File.Exists(targetpath)) {
				using (var ms = new FileStream(targetpath, FileMode.Open, FileAccess.Read, FileShare.Read).CopyToMemoryAndDispose()) {
					ms.Position = 0;
					if (SenUtils.CalcSha1(ms) == sha1) {
						// target already has the correct file, nothing to do
						return true;
					}
				}
			}

			// file doesn't exist or mismatches
			Stream stream = DoCreate();
			if (SenUtils.CalcSha1(stream) != sha1) {
				throw new Exception("Created incorrect file.");
			}
			return SenUtils.TryWriteFileIfDifferent(stream, targetpath);
		}

		public bool TryRevert(string basepath, string backuppath) {
			string subtargetpath = GetSubTargetPath();
			string sha1 = GetTargetSha1();
			string targetpath = Path.Combine(basepath, subtargetpath);

			if (File.Exists(targetpath)) {
				bool dodelete = false;
				using (var ms = new FileStream(targetpath, FileMode.Open, FileAccess.Read, FileShare.Read).CopyToMemoryAndDispose()) {
					ms.Position = 0;
					if (SenUtils.CalcSha1(ms) == sha1) {
						dodelete = true;
					}
				}

				if (dodelete) {
					return SenUtils.TryDeleteFile(targetpath);
				}

				return false;
			}

			return true;
		}

		public abstract string GetSubTargetPath();

		public abstract string GetTargetSha1();

		protected abstract Stream DoCreate();

		public abstract string GetDescription();
	}
}
