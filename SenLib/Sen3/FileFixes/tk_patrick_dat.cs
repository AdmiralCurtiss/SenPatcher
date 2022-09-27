using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class tk_patrick_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in conversations with Patrick.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6ed1ba8558121db3ul, 0x18927df877171223ul, 0x3f41fcceu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// !? -> ?!
			bin.SwapBytes(0x2ff, 0x300);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_patrick.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6ed1ba8558121db3ul, 0x18927df877171223ul, 0x3f41fcceu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_en/tk_patrick.dat", file) };
		}
	}
}
