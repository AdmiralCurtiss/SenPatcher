using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_vandyck_dat : FileMod {
		public string GetDescription() {
			return "Fix double space in conversation with Vandyck.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb838141d25f707a7ul, 0xc95191db2f8c324aul, 0x3e0a34c0u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1312, 0x183, 0x1420, 1);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_vandyck.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb838141d25f707a7ul, 0xc95191db2f8c324aul, 0x3e0a34c0u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_vandyck.dat", s) };
		}
	}
}
