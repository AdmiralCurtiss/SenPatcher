using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0800_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in southern Nord plains.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x48d8e5ebfacb29feul, 0xd7c5c4e75c84e108ul, 0x277cb5feu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// double space
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0xf877, 0x46, 0xf89c, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0800.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x48d8e5ebfacb29feul, 0xd7c5c4e75c84e108ul, 0x277cb5feu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0800.dat", s) };
		}
	}
}
