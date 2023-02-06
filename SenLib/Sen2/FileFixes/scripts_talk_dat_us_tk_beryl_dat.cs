using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_talk_dat_us_tk_beryl_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in conversation with Beryl.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x73f84be48f549fb7ul, 0x1ac265a5d7519f3dul, 0x8ca16f1du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			patcher.ExtendPartialCommand(0xfdd, 0xac, 0xffb, new byte[] {0x68});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_beryl.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x73f84be48f549fb7ul, 0x1ac265a5d7519f3dul, 0x8ca16f1du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_beryl.dat", file) };
		}
	}
}
