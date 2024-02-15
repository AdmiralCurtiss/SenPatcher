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
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd1cd26a05828553bul, 0xb7bf03e370717226ul, 0xf28353a0u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			// var patcher = new SenScriptPatcher(bin);

			// astrology -> astronomy in cocktail with Towa
			bin.Position = 0x39b68;
			bin.Write(new byte[] {0x6e, 0x6f, 0x6d});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f4200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd1cd26a05828553bul, 0xb7bf03e370717226ul, 0xf28353a0u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f4200.dat", file) };
		}
	}
}
