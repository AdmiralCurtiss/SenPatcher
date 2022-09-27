using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m4004_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Gral of Erebos (Bottom).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5941f93f60a4ab76ul, 0x661ae37e51103b8ful, 0x1cf35011u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Sept-Terrion of Flame -> Sept-Terrion of Fire
			patcher.ReplacePartialCommand(0x33b7, 0x82, 0x33e3, 0x3, new byte[] {0x69, 0x72});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m4004.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5941f93f60a4ab76ul, 0x661ae37e51103b8ful, 0x1cf35011u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m4004.dat", file) };
		}
	}
}
