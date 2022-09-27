using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m0000_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Einhel Lv0.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x289204f27043cf2ful, 0xdc6272bffd1ec085ul, 0x7f3230d3u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synch -> sync
			patcher.RemovePartialCommand(0x9001, 0x4a, 0x9032, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x289204f27043cf2ful, 0xdc6272bffd1ec085ul, 0x7f3230d3u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0000.dat", file) };
		}
	}
}
