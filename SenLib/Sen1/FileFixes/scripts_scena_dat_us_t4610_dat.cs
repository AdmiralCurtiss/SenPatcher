using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_t4610_dat : FileMod {
		public string GetDescription() {
			return "Typo fix in Gwyn's cabin.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xee2a5c698325223eul, 0xf9aaf196af96e4a5ul, 0x5dd18f16u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// typo fix, Looks what way -> Looks that way
			bin.Position = 0x6e5;
			bin.WriteByte(0x74);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4610.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xee2a5c698325223eul, 0xf9aaf196af96e4a5ul, 0x5dd18f16u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4610.dat", file) };
		}
	}
}
