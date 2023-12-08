using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_e7090_dat : FileMod {
		public string GetDescription() {
			return "Add unused grunt to Jusis' line in scene on Courageous after Celdic.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x34005eaf8e8f7823ul, 0xf5079700cf130b2bul, 0x3b73d047u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// add Jusis grunt voice clip
			patcher.ExtendPartialCommand(0x161bb, 0xd, 0x161c3, new byte[] { 0x11, 0x50, 0xff, 0x00, 0x00 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7090.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x34005eaf8e8f7823ul, 0xf5079700cf130b2bul, 0x3b73d047u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7090.dat", file) };
		}
	}
}
