using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_e7060_dat : FileMod {
		public string GetDescription() {
			return "Fix text/voice inconsistency at start of Act 2 (farewell scene with Sharon).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8000797757b16c64ul, 0x656b0403584bafa1ul, 0xe85af342u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// I feel like I have so much more{n}that I can learn from you. -> I feel like I have so much more{n}I could learn from you.
			// to match VO (two instances)
			patcher.ReplacePartialCommand(0x8b90, 0xda, 0x8c4e, 0xa, new byte[] {0x49, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64});
			patcher.ReplacePartialCommand(0x2435e, 0xda, 0x2441c, 0xa, new byte[] {0x49, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7060.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8000797757b16c64ul, 0x656b0403584bafa1ul, 0xe85af342u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7060.dat", file) };
		}
	}
}
