using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_m2130_dat : FileMod {
		public string GetDescription() {
			return "Text/Voice match in Heimdallr catacombs.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa2fcccff0c7877e4ul, 0xb565ed931b62f6a0ul, 0x3191c289u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// remove stutter from text that isn't in voice
			patcher.RemovePartialCommand(0x4865, 0x38, 0x486f, 0x3);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m2130.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa2fcccff0c7877e4ul, 0xb565ed931b62f6a0ul, 0x3191c289u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m2130.dat", file) };
		}
	}
}
