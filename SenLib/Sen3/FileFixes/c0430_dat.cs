using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0430_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (Business Owners' Association).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2a1b99c4395efd23ul, 0x0bc10942ae1c06f4ul, 0x36f4e504u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Business Owner's Association -> Business Owners' Association
			bin.SwapBytes(0xd40, 0xd41);
			bin.SwapBytes(0x1493, 0x1494);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0430.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2a1b99c4395efd23ul, 0x0bc10942ae1c06f4ul, 0x36f4e504u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0430.dat", file) };
		}
	}
}
