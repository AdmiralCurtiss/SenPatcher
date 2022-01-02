using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0610_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in eastern Trista outskirts.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb96565a04c292ef7ul, 0xde28bbf071c5eae2ul, 0x2dddfffeu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// formatting issues in Sara Chapter 4 Day bonding event
			bin.SwapBytes(0x18245, 0x1824d);
			bin.SwapBytes(0x182ac, 0x182b0);
			bin.SwapBytes(0x184d6, 0x184db);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0610.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb96565a04c292ef7ul, 0xde28bbf071c5eae2ul, 0x2dddfffeu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0610.dat", s) };
		}
	}
}
