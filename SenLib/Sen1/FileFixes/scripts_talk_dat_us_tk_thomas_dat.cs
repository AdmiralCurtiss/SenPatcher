using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_talk_dat_us_tk_thomas_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in conversation with Thomas.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x465873e53df8cb85ul, 0xe26d302a171eb4ccul, 0x1ff2d6bbu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Byronia -> Bryonia
			bin.SwapBytes(0x54fd, 0x54fe);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_thomas.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x465873e53df8cb85ul, 0xe26d302a171eb4ccul, 0x1ff2d6bbu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_thomas.dat", file) };
		}
	}
}
