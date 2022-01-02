using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0010_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in Thors (bottom floor).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0xfbbf, 0x53, 0xfbcf, 1);

			// formatting issues in Towa Chapter 3 Day bonding event
			bin.SwapBytes(0x292ef, 0x292f4);
			bin.SwapBytes(0x29cf4, 0x29cf8);
			bin.Position = 0x29d48;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x29e57, 0x29e5b);
			bin.SwapBytes(0x29e78, 0x29e7e);
			bin.SwapBytes(0x2a13e, 0x2a143);
			bin.SwapBytes(0x2a3fc, 0x2a3ff);
			bin.SwapBytes(0x2a421, 0x2a42a);
			bin.SwapBytes(0x2a6e3, 0x2a6e6);
			bin.SwapBytes(0x2a72a, 0x2a72f);

			// formatting issues in Fie Chapter 4 Day bonding event
			bin.SwapBytes(0x2b045, 0x2b04a);
			bin.SwapBytes(0x2b069, 0x2b070);

			// formatting issues in Fie Chapter 5 Evening bonding event
			bin.SwapBytes(0x2bda3, 0x2bda7);
			bin.SwapBytes(0x2bdc6, 0x2bdcb);
			bin.SwapBytes(0x2c02e, 0x2c032);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", s) };
		}
	}
}
