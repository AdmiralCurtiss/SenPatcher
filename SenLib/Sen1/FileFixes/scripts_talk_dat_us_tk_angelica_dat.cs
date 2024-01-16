using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_talk_dat_us_tk_becky_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in conversation with Becky.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3eb462def36cc151ul, 0x03e48657aa18b6b0ul, 0x2abcd830u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			bin.Position = 0x13e;
			bin.WriteByte(0x61);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_becky.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3eb462def36cc151ul, 0x03e48657aa18b6b0ul, 0x2abcd830u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_becky.dat", file) };
		}
	}
}
