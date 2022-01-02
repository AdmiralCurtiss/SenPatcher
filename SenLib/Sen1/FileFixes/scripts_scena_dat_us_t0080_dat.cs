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

			// formatting issues in Jusis Chapter 4 Day bonding event
			bin.SwapBytes(0x2c35f, 0x2c362);
			bin.SwapBytes(0x2c467, 0x2c46a);
			bin.SwapBytes(0x2c4dd, 0x2c4e2);

			// formatting issues in Emma Chapter 4 Day bonding event
			bin.Position = 0x303f1;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x3042b, 0x3042f);
			bin.SwapBytes(0x305c0, 0x305c4);
			bin.SwapBytes(0x30b71, 0x30b75);
			bin.SwapBytes(0x30b96, 0x30b9f);
			bin.SwapBytes(0x30df1, 0x30dfc);
			bin.Position = 0x30e17;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x30f4e, 0x30f53);

			// formatting issues in Machias Chapter 5 Evening bonding event
			bin.SwapBytes(0x2e4ca, 0x2e4cd);
			bin.SwapBytes(0x2efa4, 0x2efa8);
			bin.SwapBytes(0x2fd55, 0x2fd61);

			// formatting issues in Emma/Fie midterms event
			bin.SwapBytes(0x33b77, 0x33b7c);

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
