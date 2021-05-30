using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book00_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 in Imperial Chronicle back-issues.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7a68747acbd132c5ul, 0x215ea9c7543c37f1ul, 0x46b39d63u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// TODO

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7a68747acbd132c5ul, 0x215ea9c7543c37f1ul, 0x46b39d63u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", s) };
		}
	}
}
