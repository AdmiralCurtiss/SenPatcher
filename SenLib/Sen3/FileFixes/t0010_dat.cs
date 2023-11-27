using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0010_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Branch Campus Dorm.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x53812a2a94c94c7bul, 0xc9068c4e43d8973eul, 0xb32d5034u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Master Yun -> Master Ka-fai
			patcher.ReplacePartialCommand(0x1be3a, 0x50, 0x1be84, 0x3, new byte[] {0x4b, 0x61, 0x2d, 0x66, 0x61, 0x69});

			// Hah. Guess so... -> Hmph. True. (to match existing voice line)
			patcher.ReplacePartialCommand(0x39b4e, 0x1e, 0x39b5b, 0xe, new byte[] {0x6d, 0x70, 0x68, 0x2e, 0x20, 0x54, 0x72, 0x75, 0x65});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x53812a2a94c94c7bul, 0xc9068c4e43d8973eul, 0xb32d5034u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0010.dat", file) };
		}
	}
}
