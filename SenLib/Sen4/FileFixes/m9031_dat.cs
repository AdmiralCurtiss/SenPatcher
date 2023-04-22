using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
	class m9031_dat : FileMod {
		public string GetDescription() {
			return "Newline fix in final dungeon.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xcdc0b7a54ba0420aul, 0x0c7c314d2a021f69ul, 0x49f5cb2eu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			bin.SwapBytes(0x7d13, 0x7d1d);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m9031.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xcdc0b7a54ba0420aul, 0x0c7c314d2a021f69ul, 0x49f5cb2eu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m9031.dat", file) };
		}
	}
}
