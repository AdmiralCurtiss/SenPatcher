using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0210_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Branch Campus Main Building.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc45eb8312b11f6a8ul, 0x7476165fe69d6467ul, 0x89d5e48bu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Intelligence Agency -> Intelligence Division
			bin.Position = 0x1e848;
			patcher.ReplacePartialCommand(0x1ec86, 0x9a, 0x1ed0c, 0x6, bin.ReadBytes(8));

			// synch -> sync
			patcher.RemovePartialCommand(0x4b1e8, 0x48, 0x4b213, 0x1);

			// PS4 patch script has two more changes here, which I believe are both incorrect.
			// One is the controller button icon in the tutorial message for how to use the Arcus menu,
			// which appears to already work correctly in the PC version.
			// The other one is capitalizing Port City (which they've done in lots of other places too),
			// but based on the context I think this one doesn't make any sense. It's not referring to
			// Ordis in particular, but rather port cities in general (the line is "That's the biggest
			// port city on the west coast, right?"), so I think leaving it lowercase makes more sense.

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0210.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc45eb8312b11f6a8ul, 0x7476165fe69d6467ul, 0x89d5e48bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0210.dat", file) };
		}
	}
}
