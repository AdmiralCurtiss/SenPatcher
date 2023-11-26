using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c3000_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Heimdallr (Sankt District).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6c8a21013585ce0ful, 0x1ddf378cf8963aecul, 0x5f283a75u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Miss Clotilde -> Ms. Clotilde
			patcher.ReplacePartialCommand(0x1c3f6, 0x4c, 0x1c433, 0x3, new byte[] {0x73, 0x2e});

			// Sir Hemisphere -> Father Wazy
			patcher.ReplacePartialCommand(0x25615, 0x89, 0x25648, 0xe, new byte[] {0x46, 0x61, 0x74, 0x68, 0x65, 0x72, 0x20, 0x57, 0x61, 0x7a, 0x79});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6c8a21013585ce0ful, 0x1ddf378cf8963aecul, 0x5f283a75u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3000.dat", file) };
		}
	}
}
