using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_edel_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in conversation with Edel.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc5deaa30247aafaul, 0xf2a369da2268d408ul, 0x2ba310c5u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x3179, 0x48, 0x31aa, 1);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_edel.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdc5deaa30247aafaul, 0xf2a369da2268d408ul, 0x2ba310c5u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_edel.dat", s) };
		}
	}
}
