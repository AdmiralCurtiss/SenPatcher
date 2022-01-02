using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1020_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Brandon's.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5574310597160a8bul, 0x94e2a8ccf2ad2dfdul, 0xc22c79d2u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Laura Chapter 3 Day bonding event
			bin.SwapBytes(0x575c, 0x5762);
			bin.SwapBytes(0x5d6b, 0x5d6f);
			bin.SwapBytes(0x60aa, 0x60ae);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1020.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5574310597160a8bul, 0x94e2a8ccf2ad2dfdul, 0xc22c79d2u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1020.dat", s) };
		}
	}
}
