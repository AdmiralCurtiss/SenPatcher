using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_talk_dat_us_tk_angelica_dat : FileMod {
		public string GetDescription() {
			return "Fix extra word in conversation with Angelica.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe5f61f3108f8e1d4ul, 0x3ca539b93bac91cful, 0xe37b709au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// remove extra 'to'
			// this should be right but I can't find this line in-game, so let's disable it for now...
			//patcher.RemovePartialCommand(0x619, 0x8f, 0x676, 0x3);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_angelica.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe5f61f3108f8e1d4ul, 0x3ca539b93bac91cful, 0xe37b709au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_angelica.dat", file) };
		}
	}
}
