using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_us_t_magic_tbl : FileMod {
		public string GetDescription() {
			return "Fix typo in Emma's S-Craft.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			bin.SwapBytes(0x3c3a, 0x3c3c);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", s) };
		}
	}
}
