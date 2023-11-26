using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c2440_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Valflame Palace (back area).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x37161993d10c6582ul, 0x1c11026fedb1af52ul, 0x90ceff4du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Highness -> Majesty
			patcher.ReplacePartialCommand(0x30a9, 0x47, 0x30e2, 0x8, new byte[] {0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}); // rean to eugent
			patcher.ReplacePartialCommand(0x52f6, 0xd4, 0x5393, 0x8, new byte[] {0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}); // rean about eugent

			// royal family -> Imperial family
			patcher.ReplacePartialCommand(0x45d8, 0x1c2, 0x46ce, 0x3, new byte[] {0x49, 0x6d, 0x70, 0x65, 0x72, 0x69});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2440.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x37161993d10c6582ul, 0x1c11026fedb1af52ul, 0x90ceff4du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2440.dat", file) };
		}
	}
}
