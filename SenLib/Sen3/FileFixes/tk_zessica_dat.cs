using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_zessica_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in conversation with Jessica.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a9a5ca2e438e36eul, 0xa47c147124623407ul, 0xae9b6b5bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Nevermind -> Never mind
			patcher.ExtendPartialCommand(0x1bec, 0x9e, 0x1c83, new byte[] {0x20});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_zessica.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a9a5ca2e438e36eul, 0xa47c147124623407ul, 0xae9b6b5bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_zessica.dat", file) };
		}
	}
}

