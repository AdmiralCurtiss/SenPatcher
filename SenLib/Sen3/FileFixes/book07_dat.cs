using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class book07_dat : FileMod {
		public string GetDescription() {
			return "Fix typos in Black Records.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4531fd2f226f5c64ul, 0x1e09fc35e118d1c1ul, 0xbb2a4144u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// great knight -> Great Knight
			bin.Position = 0x40a1;
			bin.WriteUInt8(0x47);
			bin.Position = 0x40a7;
			bin.WriteUInt8(0x4b);

			// TODO: Port the CS2 fixes here

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book07.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4531fd2f226f5c64ul, 0x1e09fc35e118d1c1ul, 0xbb2a4144u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book07.dat", file) };
		}
	}
}
