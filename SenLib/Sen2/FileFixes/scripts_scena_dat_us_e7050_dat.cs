using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_e7050_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in Grianos' name.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd0a9a39e3aa04d57ul, 0x3b9a7b51a170fb1dul, 0xc4d79f17u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Girianos -> Grianos
			patcher.RemovePartialCommand(0x9369, 0xbb, 0x93de, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7050.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd0a9a39e3aa04d57ul, 0x3b9a7b51a170fb1dul, 0xc4d79f17u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/e7050.dat", file) };
		}
	}
}
