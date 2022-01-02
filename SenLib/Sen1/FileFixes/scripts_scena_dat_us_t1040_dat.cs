using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1040_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in Le Sage (Trista).";
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

			// formatting issues in Millium Chapter 5 Day bonding event
			bin.SwapBytes(0x5ab0, 0x5ab4);
			bin.SwapBytes(0x5ad6, 0x5ada);
			bin.SwapBytes(0x5b1f, 0x5b26);
			bin.Position = 0x5b5d;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x5bde, 0x5be6);
			bin.SwapBytes(0x5d8c, 0x5d91);
			bin.SwapBytes(0x5dd3, 0x5dd8);
			bin.SwapBytes(0x5e3d, 0x5e42);
			bin.SwapBytes(0x67dd, 0x67e3);
			bin.SwapBytes(0x68b5, 0x68ba);

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
