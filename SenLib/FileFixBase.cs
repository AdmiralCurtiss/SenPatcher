using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public abstract class FileFixBase : FileFix {
		public bool TryApply(string basepath) {
			string subpath = GetSubPath();
			string bkpsubpath = GetBackupSubPath();
			string sha1 = GetSha1();

			string path = Path.Combine(basepath, subpath);
			string bkppath = Path.Combine(basepath, bkpsubpath);
			using (var ms = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read).CopyToMemoryAndDispose()) {
				ms.Position = 0;
				if (SenUtils.CalcSha1(ms) == sha1) {
					// original file is unpatched, copy to backup
					SenUtils.CreateBackupIfRequired(bkppath, ms);

					// and apply
					DoApplyAndWrite(path, ms);
					return true;
				}
			}

			// check if backup path has a clean copy
			if (File.Exists(bkppath)) {
				using (var ms = new FileStream(bkppath, FileMode.Open, FileAccess.Read, FileShare.Read).CopyToMemoryAndDispose()) {
					if (SenUtils.CalcSha1(ms) == sha1) {
						DoApplyAndWrite(path, ms);
						return true;
					}
				}
			}

			// we have no clean copy
			return false;
		}

		public abstract string GetSubPath();

		public virtual string GetBackupSubPath() {
			return Path.Combine(SenCommonPaths.BackupFolder, GetSubPath().Replace('/', '_').Replace('\\', '_'));
		}

		public abstract string GetSha1();

		private void DoApplyAndWrite(string path, Stream bin) {
			DoApply(bin);
			bin.Position = 0;
			using (var fs = new FileStream(path, FileMode.Create)) {
				StreamUtils.CopyStream(bin, fs);
			}
		}

		protected abstract void DoApply(Stream bin);
	}
}
