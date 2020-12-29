using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class text_dat_us_t_magic_tbl : FileMod {
		public string GetDescription() {
			return "Fix typo in Emma's S-Craft.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// fix typo in Emma's S-Craft
			bin.SwapBytes(0x642b, 0x642d);

			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", s) };
		}
	}
}
