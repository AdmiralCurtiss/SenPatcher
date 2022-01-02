using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1500_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues and text/voice mismatch in Celdic.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4f2a0d4b4c1602bbul, 0xd9aec8dc0785f334ul, 0xc6c285f9u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x11129, 0x34, 0x11136, 3, new byte[] { 0x27 });
			patcher.ReplacePartialCommand(0x2fc1d, 0xf, 0x2fc25, 2, new byte[] { 0x52, 0x2d, 0x52, 0x69, 0x67, 0x68, 0x74 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1500.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4f2a0d4b4c1602bbul, 0xd9aec8dc0785f334ul, 0xc6c285f9u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1500.dat", s) };
		}
	}
}
