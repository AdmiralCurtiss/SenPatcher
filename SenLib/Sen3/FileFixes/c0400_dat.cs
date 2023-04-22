using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0400_dat : FileMod {
		public string GetDescription() {
			return "Typo in Crossbell East Street.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x91fe1bb9b0861e93ul, 0xa37835c7678dd389ul, 0xad365655u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// IIt's -> It's
			patcher.RemovePartialCommand(0x397b, 0xf3, 0x3983, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0400.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x91fe1bb9b0861e93ul, 0xa37835c7678dd389ul, 0xad365655u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0400.dat", file) };
		}
	}
}
