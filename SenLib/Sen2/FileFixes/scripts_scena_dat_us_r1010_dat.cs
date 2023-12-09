using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_r1010_dat : FileMod {
		public string GetDescription() {
			return "Fix missing word in Nicholas/Emily recruitment sidequest.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc21cb1a876196b75ul, 0x51f3bc3ef57620c1ul, 0xffc0deb1u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// missing 'it'
			bin.SwapBytes(0x4907, 0x490a);
			patcher.ExtendPartialCommand(0x48dd, 0x39, 0x4904, new byte[] { 0x69, 0x74, 0x20 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r1010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc21cb1a876196b75ul, 0x51f3bc3ef57620c1ul, 0xffc0deb1u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r1010.dat", file) };
		}
	}
}
