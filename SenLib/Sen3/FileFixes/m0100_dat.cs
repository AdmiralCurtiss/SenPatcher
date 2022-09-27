using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m0100_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Einhel Lv1.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb0f3a274fd1e5452ul, 0x8118b4a68706e7c3ul, 0x30285fefu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synch -> sync
			patcher.RemovePartialCommand(0x4a10, 0xc8, 0x4ac6, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0100.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb0f3a274fd1e5452ul, 0x8118b4a68706e7c3ul, 0x30285fefu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0100.dat", file) };
		}
	}
}
