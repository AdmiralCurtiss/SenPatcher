using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0010_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Thors (bottom floor).";
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
			bin.Position = 0x29d48;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x2a3fc, 0x2a3ff);
			bin.SwapBytes(0x2a421, 0x2a42a);
			bin.SwapBytes(0x2a6e3, 0x2a6e6);
			bin.SwapBytes(0x2a62a, 0x2a62f);

			// formatting issues in Fie Chapter 4 Day bonding event
			bin.SwapBytes(0x2b045, 0x2b04a);
			bin.SwapBytes(0x2b069, 0x2b070);

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
