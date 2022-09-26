using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t_place : FileMod {
		public string GetDescription() {
			return "Fix minor error in area names.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6f74b48ecea8b47eul, 0xc2d951d0cdc29f31ul, 0x3ec82738u));
			if (file_en == null) {
				return null;
			}

			var s = file_en.CopyToMemory();
			s.SwapBytes(0x11c1, 0x11c1 + 1);
			s.SwapBytes(0x3a1f, 0x3a1f + 1);

			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_place.tbl", s),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6f74b48ecea8b47eul, 0xc2d951d0cdc29f31ul, 0x3ec82738u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_place.tbl", file_en.Duplicate()),
			};
		}
	}
}
