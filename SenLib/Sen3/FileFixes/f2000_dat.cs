using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class f2000_dat : FileMod {
		public string GetDescription() {
			return "Fix Hamilton gender (scene 2, text only).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xbb3220cb9f85e554ul, 0xfd1196688bcc1ea5ul, 0x78a3d234u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// switch gender in line from Angelica
			patcher.ExtendPartialCommand(0x46cf, 0x1b0, 0x482b, new byte[] { 0x73 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f2000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xbb3220cb9f85e554ul, 0xfd1196688bcc1ea5ul, 0x78a3d234u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/f2000.dat", file) };
		}
	}
}
