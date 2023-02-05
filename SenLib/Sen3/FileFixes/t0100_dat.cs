using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0100_dat : FileMod {
		public string GetDescription() {
			return "Textbox overflow in Recette Bakery.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x02b743e33e402065ul, 0xf67af2fa318064f4ul, 0x1314d353u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// chapter 1 system message for going to the einhel keep
			// swap linebreak forwards so line doesn't overflow textbox
			bin.SwapBytes(0x8a0a, 0x8a19);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0100.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x02b743e33e402065ul, 0xf67af2fa318064f4ul, 0x1314d353u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0100.dat", file) };
		}
	}
}
