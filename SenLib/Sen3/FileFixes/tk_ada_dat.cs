using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_ada_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in conversations with Ada.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4c1c3cd56be5eb1aul, 0x950812709685b2f0ul, 0x29096f2du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Miss Arseid -> Ms. Arseid (esmelas garden party, after talk with priscilla)
			patcher.ReplacePartialCommand(0x2c0, 0xaf, 0x31a, 0x3, new byte[] {0x73, 0x2e});

			// smart quote fix (same as above for first, unsure for second.
			// it should be after the 'From A-da to Frit-Z' sidequest but I have no idea where she ends up standing around there...)
			patcher.ReplacePartialCommand(0x2c0, 0xaf, 0x2da, 0x3, new byte[] {0x27});
			patcher.ReplacePartialCommand(0x4a6, 0x52, 0x4b8, 0x3, new byte[] {0x27});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_ada.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4c1c3cd56be5eb1aul, 0x950812709685b2f0ul, 0x29096f2du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_ada.dat", file) };
		}
	}
}
