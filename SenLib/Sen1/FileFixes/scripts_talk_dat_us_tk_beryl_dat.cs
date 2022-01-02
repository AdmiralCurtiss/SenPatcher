using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_beryl_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in conversation with Beryl.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xaf8f356c80083c02ul, 0x8824fd41332b003ful, 0xec834cb1u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x5b86, 0x3d, 0x5bb8, 3, new byte[] { 0x27 });

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_beryl.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xaf8f356c80083c02ul, 0x8824fd41332b003ful, 0xec834cb1u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_beryl.dat", s) };
		}
	}
}
