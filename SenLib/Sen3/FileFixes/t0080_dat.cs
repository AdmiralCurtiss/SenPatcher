using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0080_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Leeves (Radio Trista).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdacda5af52ca6ab3ul, 0xefee4f9b51606a5eul, 0x3d676a7eu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// St. Ursula's -> St. Ursula
			patcher.RemovePartialCommand(0x1115e, 0x7f, 0x111b4, 0x2);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0080.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdacda5af52ca6ab3ul, 0xefee4f9b51606a5eul, 0x3d676a7eu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0080.dat", file) };
		}
	}
}
