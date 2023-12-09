using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t0010_dat : FileMod {
		public string GetDescription() {
			return "Fix extra word in Thors (bottom floor).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7f1a94208801acaful, 0x1a3ba2fd7cba4964ul, 0x44904a1bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// during the our days in the army -> during our days in the army
			patcher.RemovePartialCommand(0x4201, 0x11b, 0x42dc, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7f1a94208801acaful, 0x1a3ba2fd7cba4964ul, 0x44904a1bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", file) };
		}
	}
}
