using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_linde_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in conversations with Linde.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65aa739d1b7a5f3ful, 0xf91e7fcf4840a5dbul, 0x26bfcdfbu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// St. Ursula's -> St. Ursula (after picking her for the radio sidequest in chapter 4)
			patcher.RemovePartialCommand(0xac3, 0xcd, 0xb54, 0x2);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_linde.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65aa739d1b7a5f3ful, 0xf91e7fcf4840a5dbul, 0x26bfcdfbu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_linde.dat", file) };
		}
	}
}
