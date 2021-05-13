using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_m0040_dat : FileMod {
		public string GetDescription() {
			return "Fix text/voice mismatch in Lunaria Nature Park.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc377fcdc74a195f7ul, 0x9dba543e80a70d2eul, 0xb7fdf12au));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x67a4, 0x16, 0x67ad, 7, new byte[] { 0x41, 0x68 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m0040.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc377fcdc74a195f7ul, 0x9dba543e80a70d2eul, 0xb7fdf12au));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m0040.dat", s) };
		}
	}
}
