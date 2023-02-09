using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_talk_dat_us_tk_heinrich_dat : FileMod {
		public string GetDescription() {
			return "Fix missing period in conversation with Vice Principal Heinrich.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1e88dbcff39609faul, 0xcaaca7a298032477ul, 0x39ff1f14u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			patcher.ExtendPartialCommand(0xed8, 0x131, 0x1007, new byte[] {0x2e});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_heinrich.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1e88dbcff39609faul, 0xcaaca7a298032477ul, 0x39ff1f14u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_heinrich.dat", file) };
		}
	}
}
