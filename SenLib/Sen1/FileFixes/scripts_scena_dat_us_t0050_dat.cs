using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0050_dat : FileMod {
		public string GetDescription() {
			return "Fix double space in Mishy Panic.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x11b02, 0x44, 0x11b10, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0050.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd363fc2114ec8421ul, 0xc24b47c29a4a2badul, 0xed31cfb5u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0050.dat", s) };
		}
	}
}
