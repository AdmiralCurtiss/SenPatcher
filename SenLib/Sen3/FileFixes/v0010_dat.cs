using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class v0010_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Derfflinger Car 1.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4c2781204c47f245ul, 0x7e831b6a50413068ul, 0xb498c5b3u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// the Juno Naval Fortress -> Juno Naval Fortress
			patcher.RemovePartialCommand(0x26097, 0xe7, 0x26147, 0x4);
			patcher.RemovePartialCommand(0x262c6, 0x25, 0x262d0, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/v0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4c2781204c47f245ul, 0x7e831b6a50413068ul, 0xb498c5b3u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/v0010.dat", file) };
		}
	}
}
