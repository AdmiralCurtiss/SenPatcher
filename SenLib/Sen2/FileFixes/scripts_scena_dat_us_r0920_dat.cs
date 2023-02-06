using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_r0920_dat : FileMod {
		public string GetDescription() {
			return "Fix logic error in Gone Air sidequest.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1e69ef573a1888e5ul, 0x545850c97d6252fbul, 0x1f5b5596u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// this is a rather fun error that I totally get how it happened
			// if Millium is in the party, the conversation goes
			// Rean: i'm impressed you figured out this stuff [about the enemy mercenaries]
			// Millium: did you get some info from the RMP? you definitely have their weapons!
			// Klein: haha, yeah you're right
			// Klein: we're working with them
			// but if Millium is NOT in the party, the line from Millium and Klein's reply to it get skipped
			// which, in the English script, gives the odd impression that Klein is working with the enemy
			// mercenaries, instead of with the RMP. this fixes that by just explicitly mentioning the RMP.
			bin.SwapBytes(0x7059, 0x7060);
			patcher.ReplacePartialCommand(0x7038, 0x33, 0x7058, 0x1, new byte[] { 0x20, 0x52, 0x4d, 0x50 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0920.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1e69ef573a1888e5ul, 0x545850c97d6252fbul, 0x1f5b5596u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0920.dat", file) };
		}
	}
}
