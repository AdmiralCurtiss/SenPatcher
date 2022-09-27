using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0200_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (Central Square).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a6e007a75ff9d1bul, 0x06cc96f67d383ff7ul, 0xde6d2eb6u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Business Owner's Association -> Business Owners' Association
			bin.SwapBytes(0x14a7b, 0x14a7c);
			bin.SwapBytes(0x14c7b, 0x14c7c);
			bin.SwapBytes(0x14d34, 0x14d35);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a6e007a75ff9d1bul, 0x06cc96f67d383ff7ul, 0xde6d2eb6u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0200.dat", file) };
		}
	}
}
