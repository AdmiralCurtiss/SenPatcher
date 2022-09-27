using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3220_dat : FileMod {
		public string GetDescription() {
			return "Aurelia name fixes in Ordis (Orbal Factory).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4cdac7c910724aa2ul, 0x7bfe5ada8fc25f9bul, 0x9118833bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// General Aurelia -> General Le Guin
			bin.Position = 0x141e;
			bin.Write(new byte[] { 0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e });
			bin.Position = 0x1e43;
			bin.Write(new byte[] { 0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3220.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4cdac7c910724aa2ul, 0x7bfe5ada8fc25f9bul, 0x9118833bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3220.dat", file) };
		}
	}
}
