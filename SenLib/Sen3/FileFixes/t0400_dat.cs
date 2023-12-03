using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0400_dat : FileMod {
		public string GetDescription() {
			return "Text fixes in Einhel Keep (Outside).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x05ebfadeed23981bul, 0x39da835f2eb17987ul, 0x7003492au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// grandpa -> old man
			// This is an editing error. The previous line (from Millium) said Grandpa Schmidt on PS4 disc but
			// got changed to Old Man Schmidt in patch 1.02. The follow-up line from Schmidt to stop calling him
			// that did not get changed, however, leading to a weird mismatch.
			bin.Position = 0x4e07;
			bin.Write(new byte[] { 0x6f, 0x6c, 0x64, 0x20, 0x6d, 0x61, 0x6e });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0400.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x05ebfadeed23981bul, 0x39da835f2eb17987ul, 0x7003492au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0400.dat", file) };
		}
	}
}
