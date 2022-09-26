using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t_jump : FileMod {
		public string GetDescription() {
			return "Fix minor error in fast-travel menu text.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x57dd3ac63f52d21bul, 0x677d5e1e7c20a7f2ul, 0x55d0689fu));
			if (file_en == null) {
				return null;
			}

			var s = file_en.CopyToMemory();
			s.SwapBytes(0x1b7d, 0x1b7d + 1);

			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_jump.tbl", s),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x57dd3ac63f52d21bul, 0x677d5e1e7c20a7f2ul, 0x55d0689fu));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_jump.tbl", file_en.Duplicate()),
			};
		}
	}
}
