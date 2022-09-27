using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class f0010_dat : FileMod {
		public string GetDescription() {
			return "Aurelia name fix in Dreknor Fortress.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1c5d1ffde19b3fe0ul, 0xb14ce4b4d8c29ebaul, 0xe850b8cbu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// General Aurelia -> General Le Guin
			bin.Position = 0x2330;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1c5d1ffde19b3fe0ul, 0xb14ce4b4d8c29ebaul, 0xe850b8cbu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f0010.dat", file) };
		}
	}
}
