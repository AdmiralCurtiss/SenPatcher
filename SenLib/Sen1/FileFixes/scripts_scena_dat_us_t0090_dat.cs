using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_t0090_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in Engineering Building.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa823fb0d4b8a4ffbul, 0xdeaa6eb407bb3880ul, 0x7048b226u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// typo fix, lession -> lesson (Start of Chapter 6, 9/18, conversation with Crow/Angelica/George)
			patcher.RemovePartialCommand(0x56be, 0x75, 0x56ff, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0090.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa823fb0d4b8a4ffbul, 0xdeaa6eb407bb3880ul, 0x7048b226u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0090.dat", file) };
		}
	}
}
