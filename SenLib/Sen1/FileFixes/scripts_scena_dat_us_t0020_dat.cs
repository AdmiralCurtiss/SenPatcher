using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0020_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Thors (top floor).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9668b944717fe228ul, 0x3a482367f1448ee1ul, 0xfc63e832u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Gaius Chapter 2 Day bonding event
			bin.SwapBytes(0x58760, 0x58764);
			bin.SwapBytes(0x58915, 0x58919);
			bin.SwapBytes(0x589c3, 0x589c8);
			bin.SwapBytes(0x589f9, 0x589fd);
			bin.SwapBytes(0x58a20, 0x58a26);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0020.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9668b944717fe228ul, 0x3a482367f1448ee1ul, 0xfc63e832u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0020.dat", s) };
		}
	}
}
