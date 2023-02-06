using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_talk_dat_us_tk_emily_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in conversation with Emily.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xea9e2d4575e33478ul, 0x2fde96e3fa4d0c3ful, 0xd5545af6u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			patcher.RemovePartialCommand(0x1199, 0x6c, 0x11f7, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_emily.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xea9e2d4575e33478ul, 0x2fde96e3fa4d0c3ful, 0xd5545af6u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_emily.dat", file) };
		}
	}
}
