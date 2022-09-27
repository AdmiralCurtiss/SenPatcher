using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m3000_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Juno Naval Fortress (Sub 1).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x03bec649d56e0487ul, 0x72de4fd1ce013005ul, 0xb651d784u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// General Aurelia -> General Le Guin
			bin.Position = 0x6847;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x03bec649d56e0487ul, 0x72de4fd1ce013005ul, 0xb651d784u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m3000.dat", file) };
		}
	}
}
