using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1030_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Keynes'.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x954a31dd3eaf244ful, 0x159fbe83607ae870ul, 0xf3bdc89fu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Emma Chapter 2 Day bonding event
			bin.SwapBytes(0x4db7, 0x4dbc);
			bin.SwapBytes(0x4de2, 0x4de6);
			bin.Position = 0x5046;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x507b, 0x5080);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1030.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x954a31dd3eaf244ful, 0x159fbe83607ae870ul, 0xf3bdc89fu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1030.dat", s) };
		}
	}
}
