using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_tovar_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in conversations with Toval.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5f392e50537f7ef7ul, 0x79cb276c8d04c4ebul, 0x7b7198d4u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// smart quote fix (chapter 4 festival, horse race track lounge)
			patcher.ReplacePartialCommand(0x182, 0x33, 0x197, 0x3, new byte[] {0x27});
			patcher.ReplacePartialCommand(0x182, 0x33, 0x191, 0x3, new byte[] {0x27});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_tovar.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5f392e50537f7ef7ul, 0x79cb276c8d04c4ebul, 0x7b7198d4u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_tovar.dat", file) };
		}
	}
}
