using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3600_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Ordis (Harbor District).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf20f8c36a44c88feul, 0xcc44155250b42fa0ul, 0x259cd699u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// the Juno Naval Fortress -> Juno Naval Fortress
			patcher.RemovePartialCommand(0xb336, 0x17e, 0xb3b6, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3600.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf20f8c36a44c88feul, 0xcc44155250b42fa0ul, 0x259cd699u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3600.dat", file) };
		}
	}
}
