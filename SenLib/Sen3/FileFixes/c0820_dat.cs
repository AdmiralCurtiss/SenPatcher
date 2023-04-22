using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0820_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (IBC).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa5da1241a9c17b2ful, 0xbb2fc9c93dc691d5ul, 0x1471ec8au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// St. Ursula's hospital -> St. Ursula Medical College
			patcher.ReplacePartialCommand(0x85d6, 0xaa, 0x8606, 0xb, new byte[] {0x20, 0x4d, 0x65, 0x64, 0x69, 0x63, 0x61, 0x6c, 0x20, 0x43, 0x6f, 0x6c, 0x6c, 0x65, 0x67, 0x65});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0820.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa5da1241a9c17b2ful, 0xbb2fc9c93dc691d5ul, 0x1471ec8au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0820.dat", file) };
		}
	}
}
