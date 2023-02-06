using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t6500_dat : FileMod {
		public string GetDescription() {
			return "Missing voice clip at Schwarz Drache Barrier.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa7bff27d9558648cul, 0xb36dde4ab7cf4e9bul, 0x1ca120c2u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// "Your Lordship!" line doesn't have its voice clip, probably because it was accidentally assigned to Rean in a different scene
			patcher.ExtendPartialCommand(0x5f8f, 0x1e, 0x5f92, new byte[] {0x11, 0x19, 0xff, 0x00, 0x00});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t6500.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa7bff27d9558648cul, 0xb36dde4ab7cf4e9bul, 0x1ca120c2u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t6500.dat", file) };
		}
	}
}
