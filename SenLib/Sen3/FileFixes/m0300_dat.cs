using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m0300_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Einhel Lv3.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7103fd901920a786ul, 0x37b5abe65b071220ul, 0xd0962783u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synch -> sync
			patcher.RemovePartialCommand(0x5118, 0x98, 0x516f, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0300.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7103fd901920a786ul, 0x37b5abe65b071220ul, 0xd0962783u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0300.dat", file) };
		}
	}
}
