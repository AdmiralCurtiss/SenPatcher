using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book01_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 for Carnelia.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x033e04f8a870f076ul, 0x4460bf5e91560499ul, 0x3c035bf5u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// TODO

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book01.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x033e04f8a870f076ul, 0x4460bf5e91560499ul, 0x3c035bf5u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book01.dat", s) };
		}
	}
}
