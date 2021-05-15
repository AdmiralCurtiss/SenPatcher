using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0000_dat : FileMod {
		public string GetDescription() {
			return "Fix wrong apostrophe on Thors campus grounds.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x1c6e0, 0x8b, 0x1c71f, 3, new byte[] { 0x27 });
			patcher.ReplacePartialCommand(0x1c6e0, 0x89, 0x1c74f, 3, new byte[] { 0x27 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000.dat", s) };
		}
	}
}
