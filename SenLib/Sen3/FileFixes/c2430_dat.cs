using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c2430_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Valflame Palace (Esmelas Garden).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2fae040a29bafe3cul, 0xf202c2d5eddadef2ul, 0x424523d8u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Imperial Household Agency -> City Hall
			bin.SwapBytes(0x10560, 0x1056c); // switch line break position in following replace line
			patcher.ReplacePartialCommand(0x103e7, 0x1ab, 0x1053e, 0x1d, new byte[] {0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c}); // eugent
			patcher.ReplacePartialCommand(0x1fcbc, 0xe2, 0x1fcf6, 0x1d, new byte[] {0x43, 0x69, 0x74, 0x79, 0x20, 0x48, 0x61, 0x6c, 0x6c}); // rufus

			// Highness -> Majesty
			patcher.ReplacePartialCommand(0xa524, 0x68, 0xa542, 0x8, new byte[] {0x4d, 0x61, 0x6a, 0x65, 0x73, 0x74, 0x79}); // guard to eugent

			// royal family -> Imperial family
			patcher.ReplacePartialCommand(0xa155, 0x135, 0xa1aa, 0x3, new byte[] { 0x49, 0x6d, 0x70, 0x65, 0x72, 0x69 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2430.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2fae040a29bafe3cul, 0xf202c2d5eddadef2ul, 0x424523d8u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c2430.dat", file) };
		}
	}
}
