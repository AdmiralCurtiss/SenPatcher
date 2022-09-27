using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c3610_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Karel Imperial Villa (endgame).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x043cfd82b96630f5ul, 0x6a838aadce36591dul, 0x2001196fu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synched -> synced
			patcher.RemovePartialCommand(0x15249, 0x3d, 0x15264, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3610.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x043cfd82b96630f5ul, 0x6a838aadce36591dul, 0x2001196fu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3610.dat", file) };
		}
	}
}
