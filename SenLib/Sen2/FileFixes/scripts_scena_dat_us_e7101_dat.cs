using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_scena_dat_us_e7101_dat : FileMod {
		public string GetDescription() {
			return "Fix incorrect voice clip being used near end of Intermission.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xcc2ff5a5eadebb18ul, 0xa6db19106284dd35ul, 0xac201243u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x107b, 0x18, 0x1083, 1);
			patcher.RemovePartialCommand(0x64e9, 0x22, 0x64f4, 5);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7101.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xcc2ff5a5eadebb18ul, 0xa6db19106284dd35ul, 0xac201243u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7101.dat", s) };
		}
	}
}
