using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_laura_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in conversation with Laura.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf423fb1dfddde29dul, 0x3e26a40ceed87982ul, 0xb899cdcau));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1881, 0x73, 0x18a7, 1);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_laura.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf423fb1dfddde29dul, 0x3e26a40ceed87982ul, 0xb899cdcau));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_laura.dat", s) };
		}
	}
}
