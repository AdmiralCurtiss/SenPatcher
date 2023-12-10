using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t3500_dat : FileMod {
		public string GetDescription() {
			return "Text/Voice match in Roer.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3f981082b2affc86ul, 0x9785a08896aad11aul, 0x50c44343u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Huh? -> Hmm?
			// This is in Act 2, early into the Roer section, right before reuniting with Angelica.
			bin.Position = 0x17e54;
			bin.Write(new byte[] {0x6d, 0x6d});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3500.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3f981082b2affc86ul, 0x9785a08896aad11aul, 0x50c44343u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3500.dat", file) };
		}
	}
}
