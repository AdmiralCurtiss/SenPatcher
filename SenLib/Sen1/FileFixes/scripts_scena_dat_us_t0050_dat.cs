using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0050_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in the Gymnasium.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// double space
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x11b02, 0x44, 0x11b10, 1);

			// formatting issues in Laura Chapter 1 Day bonding event
			bin.SwapBytes(0x14e92, 0x14e95);
			bin.SwapBytes(0x1505a, 0x15061);
			bin.SwapBytes(0x15186, 0x1518e);
			bin.SwapBytes(0x153a1, 0x153a6);

			// formatting issues in Laura Chapter 4 Evening bonding event
			bin.SwapBytes(0x162a6, 0x162ab);
			bin.SwapBytes(0x164ae, 0x164b4);
			bin.SwapBytes(0x164d8, 0x164dd);
			bin.SwapBytes(0x16627, 0x1662a);

			// formatting issues in Laura Chapter 5 Day bonding event
			bin.Position = 0x17070;
			bin.WriteUInt8(0x01);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0050.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0050.dat", s) };
		}
	}
}
