using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c2610_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Heimdallr (Foresta Tavern & Inn).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd89a6f84a665a650ul, 0xf32269931c874bf3ul, 0xd1cb0b1bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Fisherman, 2nd Class -> 2nd Class Fisherman
			using (MemoryStream ms = new MemoryStream()) {
				ms.Write(bin.ReadBytesFromLocationAndReset(0x4c83, 9));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x4c77, 10));
				patcher.ReplacePartialCommand(0x4bf2, 0xe2, 0x4c78, 0x14, ms.CopyToByteArrayAndDispose());
			}

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2610.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd89a6f84a665a650ul, 0xf32269931c874bf3ul, 0xd1cb0b1bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2610.dat", file) };
		}
	}
}
