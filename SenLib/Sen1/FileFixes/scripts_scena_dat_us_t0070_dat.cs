using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_t0070_dat : FileMod {
		public string GetDescription() {
			return "Minor fixes in Auditorium.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xeab3c1e32b228763ul, 0x9eab4b87c8ce67a4ul, 0x1e0c4d80u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// remove misplaced voice clip (NPC that says the same line in a cutscene where it's voiced) (Final Chapter, very last free movement sequence)
			patcher.RemovePartialCommand(0x4718, 0x31, 0x471b, 0x5);

			// flip text (but not voice) of Alisa's reactions to Angelica (from PS4) (before concert)
			// this change is pretty arguable so I don't think I'll actually use it...
			//patcher.ReplacePartialCommand(0x12ba6, 0x1e, 0x12bb4, 0x3, new byte[] { 0x2d });
			//patcher.ReplacePartialCommand(0x1374b, 0x16, 0x13754, 0x1, new byte[] { 0x68, 0x21, 0x20 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0070.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xeab3c1e32b228763ul, 0x9eab4b87c8ce67a4ul, 0x1e0c4d80u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0070.dat", file) };
		}
	}
}
