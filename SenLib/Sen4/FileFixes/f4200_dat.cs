using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
	class f4200_dat : FileMod {
		public string GetDescription() {
			return "Fix astrology/astronomy mixup.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x64a9b5e395a29f2bul, 0x62c6000ee60a8239ul, 0x0a369101u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			// var patcher = new SenScriptPatcher(bin);

			// astrology -> astronomy in cocktail with Towa
			bin.Position = 0x39b64;
			bin.Write(new byte[] {0x6e, 0x6f, 0x6d});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f4200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x64a9b5e395a29f2bul, 0x62c6000ee60a8239ul, 0x0a369101u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f4200.dat", file) };
		}
	}
}
