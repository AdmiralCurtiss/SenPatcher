using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3200_dat : FileMod {
		public string GetDescription() {
			return "Aurelia/Bardias name fixes in Ordis (North Street).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x11f838467868bad6ul, 0x1623e965fbc3c060ul, 0x7d2a3356u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Brigadier General Wallace -> Brigadier General Bardias
			bin.Position = 0x6057;
			bin.Write(new byte[] {0x42, 0x61, 0x72, 0x64, 0x69, 0x61, 0x73});

			// General Aurelia -> General Le Guin
			bin.Position = 0x60a9;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});
			bin.Position = 0x9e1b;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x11f838467868bad6ul, 0x1623e965fbc3c060ul, 0x7d2a3356u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3200.dat", file) };
		}
	}
}
