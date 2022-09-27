using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3000_dat : FileMod {
		public string GetDescription() {
			return "Aurelia name fixes in Ordis (Business District).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd0fd8367295246abul, 0x1eafede393303d34ul, 0xf7f5bcc5u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// General Aurelia -> General Le Guin
			bin.Position = 0x72ad;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});
			bin.Position = 0x7404;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd0fd8367295246abul, 0x1eafede393303d34ul, 0xf7f5bcc5u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3000.dat", file) };
		}
	}
}
