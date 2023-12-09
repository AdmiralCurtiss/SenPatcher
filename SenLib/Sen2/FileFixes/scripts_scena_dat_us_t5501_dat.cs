using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t5501_dat : FileMod {
		public string GetDescription() {
			return "Add voice clips in scene at Garrelia Fortress.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdf089163a734c202ul, 0xebb8e05491ff6537ul, 0xfb1dbff7u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// add voice clips
			patcher.ExtendPartialCommand(0x7941, 0x10, 0x794c, new byte[] { 0x11, 0x2d, 0xff, 0x00, 0x00 });
			patcher.ExtendPartialCommand(0xa8a5, 0x1a, 0xa8b1, new byte[] { 0x11, 0x5d, 0xff, 0x00, 0x00 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t5501.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdf089163a734c202ul, 0xebb8e05491ff6537ul, 0xfb1dbff7u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t5501.dat", file) };
		}
	}
}
