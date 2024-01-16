using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0000c_dat : FileMod {
		public string GetDescription() {
			return "Fix text/voice mismatch in dance scenes.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8bf43f6d4c7f0de5ul, 0xba13e4c14301da05ul, 0xbd1c919au));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// extra word in voice but not text
			patcher.ExtendPartialCommand(0xcdd1, 0x8c, 0xce58, new byte[] { 0x2c, 0x20, 0x68, 0x75, 0x68 });

			// repeated voice clip where there shouldn't be one
			patcher.RemovePartialCommand(0xd1d1, 0x3d, 0xd1d4, 5);

			// extra word in voice but not text
			patcher.ExtendPartialCommand(0x1f46b, 0x10, 0x1f476, new byte[] { 0x4d, 0x6d, 0x2d, 0x68, 0x6d });

			// 'Ditto my dad.' -> 'Ditto my father.' to match voice clip
			bin.SwapBytes(0x20a2f, 0x20a34);
			patcher.ReplacePartialCommand(0x20a03, 0x58, 0x20a14, 0x3, new byte[] { 0x66, 0x61, 0x74, 0x68, 0x65, 0x72 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000c.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8bf43f6d4c7f0de5ul, 0xba13e4c14301da05ul, 0xbd1c919au));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000c.dat", s) };
		}
	}
}
