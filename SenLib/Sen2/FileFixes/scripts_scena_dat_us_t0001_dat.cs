using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t0001_dat : FileMod {
		public string GetDescription() {
			return "Text/voice match in epilogue NG+ only scene.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9ba616b8ea524ffful, 0x05a5ac7e709f4c5ful, 0xa4561a4bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// What -> Wh-What
			// Actually on further inspection the stutter isn't really audible in game, so let's not.
			// patcher.ExtendPartialCommand(0x18ada, 0x6f, 0x18ae9, new byte[] { 0x2d, 0x57, 0x68 });

			// Remove clip on Rosine's last '...', PS4 does this and it *is* kinda out of place and also duplicated from earlier in the scene.
			patcher.RemovePartialCommand(0x1b2e3, 0x12, 0x1b2e6, 5);


			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0001.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9ba616b8ea524ffful, 0x05a5ac7e709f4c5ful, 0xa4561a4bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0001.dat", file) };
		}
	}
}
