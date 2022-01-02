using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0080_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Student Union building.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// double space
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1f278, 0x88, 0x1f2df, 1);

			// formatting issues in Towa Chapter 1 Evening bonding event
			bin.SwapBytes(0x31889, 0x3188f);
			bin.SwapBytes(0x31ac0, 0x31ac2);
			bin.SwapBytes(0x31b09, 0x31b0e);
			bin.SwapBytes(0x31b2d, 0x31b35);
			bin.SwapBytes(0x3205c, 0x32066);
			bin.SwapBytes(0x320a5, 0x320ab);
			bin.SwapBytes(0x32175, 0x32179);

			// formatting issues in Machias Chapter 3 Day bonding event
			bin.SwapBytes(0x2d214, 0x2d21d);
			bin.SwapBytes(0x2d32f, 0x2d332);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0080.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5805f2f25de668aul, 0x4ececc8f6cad0aaaul, 0xe64a3cf8u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0080.dat", s) };
		}
	}
}
