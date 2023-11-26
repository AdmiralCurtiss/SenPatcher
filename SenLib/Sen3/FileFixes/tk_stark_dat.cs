using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_stark_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in conversations with Stark.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x095615a115f3653bul, 0x2ecb59a16a756f8cul, 0x3f12b68du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// the Juno Naval Fortress -> Juno Naval Fortress (chapter 3 field exercises day 1, camp)
			patcher.RemovePartialCommand(0x5eb3, 0x152, 0x5f31, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_stark.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x095615a115f3653bul, 0x2ecb59a16a756f8cul, 0x3f12b68du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_stark.dat", file) };
		}
	}
}
