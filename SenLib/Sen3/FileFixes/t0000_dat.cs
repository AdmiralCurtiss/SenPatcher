using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0000_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Leeves.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x895e048dd4b006b1ul, 0xbda8d2434de9edfbul, 0x20142ef9u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Sir Thomas -> Father Thomas
			patcher.ReplacePartialCommand(0x2dbe1, 0x81, 0x2dbfc, 0x2, new byte[] {0x46, 0x61, 0x74, 0x68, 0x65});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x895e048dd4b006b1ul, 0xbda8d2434de9edfbul, 0x20142ef9u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0000.dat", file) };
		}
	}
}
