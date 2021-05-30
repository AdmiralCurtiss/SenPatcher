using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book03_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 in Thors library books.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xda6f79f56616cb07ul, 0x3cc7205f895b1c0dul, 0x389c0e2du));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// TODO

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xda6f79f56616cb07ul, 0x3cc7205f895b1c0dul, 0x389c0e2du));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", s) };
		}
	}
}
