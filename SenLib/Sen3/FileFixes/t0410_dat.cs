using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0410_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Einhel Keep Entrance.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4d0f1d1d3b57eb70ul, 0xde562a52f4367495ul, 0x426d1896u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synched -> synced
			patcher.RemovePartialCommand(0x6c9d, 0x5b, 0x6cdc, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0410.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4d0f1d1d3b57eb70ul, 0xde562a52f4367495ul, 0x426d1896u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0410.dat", file) };
		}
	}
}
