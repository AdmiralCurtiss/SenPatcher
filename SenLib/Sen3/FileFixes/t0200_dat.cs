using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0200_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Branch Campus grounds.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf36378e7f72b2dd7ul, 0xe49e8b363b3916bbul, 0x388ecbc1u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Your Majesty's -> Your Highness'
			bin.Position = 0x2506e;
			bin.Write(new byte[] {0x48, 0x69, 0x67, 0x68, 0x6e, 0x65, 0x73, 0x73, 0x27});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf36378e7f72b2dd7ul, 0xe49e8b363b3916bbul, 0x388ecbc1u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0200.dat", file) };
		}
	}
}
