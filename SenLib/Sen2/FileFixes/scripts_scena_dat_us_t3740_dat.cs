using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t3740_dat : FileMod {
		public string GetDescription() {
			return "Typo fixes in Roer IoT.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x985274ffb8686434ul, 0x45b91d5f76be284aul, 0xc2c79a1cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			patcher.ExtendPartialCommand(0x36bb, 0x141, 0x36ce, new byte[] {0x72});
			patcher.ExtendPartialCommand(0x3815, 0x73, 0x382f, new byte[] {0x72});
			patcher.ExtendPartialCommand(0x38cd, 0xbe, 0x3916, new byte[] {0x72});
			patcher.ExtendPartialCommand(0x39a4, 0xba, 0x39b7, new byte[] {0x72});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3740.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x985274ffb8686434ul, 0x45b91d5f76be284aul, 0xc2c79a1cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3740.dat", file) };
		}
	}
}
