using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_c0110_dat : FileMod {
		public string GetDescription() {
			return "Fix wrong apostrophe in Le Sage scene (?)";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa740904f6ad37411ul, 0x825565981daa5f59ul, 0x15b55b69u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0xb2f, 0x61, 0xb34, 3, new byte[] { 0x27 });
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/c0110.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa740904f6ad37411ul, 0x825565981daa5f59ul, 0x15b55b69u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/c0110.dat", s) };
		}
	}
}
