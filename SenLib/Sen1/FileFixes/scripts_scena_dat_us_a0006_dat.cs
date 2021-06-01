using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_a0006_dat : FileMod {
		public string GetDescription() {
			return "Fix double spaces in debug map.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x73fd289006017eabul, 0x8816636d998d21aaul, 0x8fb38d68u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x3a67, 0x6f, 0x3ac4, 1);
			patcher.RemovePartialCommand(0x3e5f, 0x5f, 0x3e8c, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/a0006.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x73fd289006017eabul, 0x8816636d998d21aaul, 0x8fb38d68u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/a0006.dat", s) };
		}
	}
}
