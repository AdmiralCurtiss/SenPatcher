using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class a0417_dat : FileMod {
		public string GetDescription() {
			return "Fix capitalization in Chapter 1 Ex. Camp";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2b0b345b51b705b7ul, 0x1ddc5f7989836dc3ul, 0x139f62f5u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Imperial government -> Imperial Government
			bin.Position = 0xfe3b;
			bin.WriteUInt8(0x47);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/a0417.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2b0b345b51b705b7ul, 0x1ddc5f7989836dc3ul, 0x139f62f5u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/a0417.dat", file) };
		}
	}
}
