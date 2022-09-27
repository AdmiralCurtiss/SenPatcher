using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m3420_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Dark Dragon's Nest (Entrance).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2eff0eb2d6a770b4ul, 0x2069380f42d7daecul, 0xdd33a96du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// the Naval Fortress -> the naval fortress
			bin.Position = 0x2759;
			bin.WriteUInt8(0x6e);
			bin.Position = 0x275f;
			bin.WriteUInt8(0x66);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3420.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2eff0eb2d6a770b4ul, 0x2069380f42d7daecul, 0xdd33a96du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3420.dat", file) };
		}
	}
}
