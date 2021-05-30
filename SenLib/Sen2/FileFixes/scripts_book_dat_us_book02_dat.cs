using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book02_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 for Red Moon Rose.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4152ba749355789cul, 0xf55a3311cd1c5527ul, 0xc1bc8de1u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// TODO

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book02.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4152ba749355789cul, 0xf55a3311cd1c5527ul, 0xc1bc8de1u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book02.dat", s) };
		}
	}
}
