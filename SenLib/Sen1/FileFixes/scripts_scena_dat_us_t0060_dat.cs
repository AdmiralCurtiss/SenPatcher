using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0060_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in Library.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2c67ece34e81dfc6ul, 0x67fd57699b818fcful, 0xd4e6b06eu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Jusis Chapter 2 Evening bonding event
			bin.SwapBytes(0x10d09, 0x10d0c);
			bin.SwapBytes(0x10e1a, 0x10e1f);

			// formatting issues in Towa Chapter 5 Evening bonding event
			bin.Position = 0x129ff;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x12f39, 0x12f3e);
			bin.SwapBytes(0x130c3, 0x130c6);
			bin.SwapBytes(0x1343f, 0x1344b);
			bin.SwapBytes(0x13465, 0x13475);
			bin.SwapBytes(0x134a7, 0x134b1);
			bin.SwapBytes(0x13861, 0x13866);
			bin.SwapBytes(0x13aac, 0x13ab1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0060.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2c67ece34e81dfc6ul, 0x67fd57699b818fcful, 0xd4e6b06eu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0060.dat", s) };
		}
	}
}
