using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class v0050_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Derfflinger Car 6.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb1f43611bbf1130ful, 0xf552405bc50d2f75ul, 0x637e49e1u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// synch -> sync
			patcher.RemovePartialCommand(0xb533, 0x185, 0xb62c, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/v0050.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb1f43611bbf1130ful, 0xf552405bc50d2f75ul, 0x637e49e1u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/v0050.dat", file) };
		}
	}
}
