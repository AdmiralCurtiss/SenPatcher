using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1040_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Le Sage (Trista).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0c8591be2e25657dul, 0xfecdb3ead94e046cul, 0x0bf08069u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Alisa Chapter 4 Day bonding event
			bin.SwapBytes(0x4b0e, 0x4b14);
			bin.SwapBytes(0x4ca7, 0x4cab);
			bin.SwapBytes(0x4cf4, 0x4cf9);
			bin.SwapBytes(0x50c8, 0x50cd);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1040.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0c8591be2e25657dul, 0xfecdb3ead94e046cul, 0x0bf08069u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1040.dat", s) };
		}
	}
}
