using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r0210_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in North Sutherland Highway 2.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb636a0c274714e93ul, 0xb74db3871e7019bful, 0x6ca39a7cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// General Aurelia -> General Le Guin
			bin.Position = 0x39a9;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});
			bin.Position = 0x3e9b;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			// Brigadier General Wallace -> Brigadier General Bardias
			bin.Position = 0x3eb9;
			bin.Write(new byte[] {0x42, 0x61, 0x72, 0x64, 0x69, 0x61, 0x73});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r0210.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb636a0c274714e93ul, 0xb74db3871e7019bful, 0x6ca39a7cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r0210.dat", file) };
		}
	}
}
