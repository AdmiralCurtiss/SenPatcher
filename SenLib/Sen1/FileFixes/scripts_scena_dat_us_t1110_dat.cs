using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1110_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Trista chapel.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x29c1901bfb4050a4ul, 0xc0f62af6129c6538ul, 0x83f4352cu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Gaius Chapter 3 Evening bonding event
			bin.SwapBytes(0xbb1b, 0xbb1f);
			bin.SwapBytes(0xbc02, 0xbc06);
			bin.SwapBytes(0xbc73, 0xbc78);
			bin.SwapBytes(0xbd92, 0xbd97);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1110.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x29c1901bfb4050a4ul, 0xc0f62af6129c6538ul, 0x83f4352cu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1110.dat", s) };
		}
	}
}
