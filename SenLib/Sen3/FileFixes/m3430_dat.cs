using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m3430_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Dark Dragon's Nest (Area 1).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a880ba00b221cdcul, 0x176eb66cabb23573ul, 0x750251d5u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// the Juno Naval Fortress -> Juno Naval Fortress
			patcher.RemovePartialCommand(0x93d4, 0x49, 0x9403, 0x4);
			patcher.RemovePartialCommand(0x9483, 0x4c, 0x94b5, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3430.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a880ba00b221cdcul, 0x176eb66cabb23573ul, 0x750251d5u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3430.dat", file) };
		}
	}
}
