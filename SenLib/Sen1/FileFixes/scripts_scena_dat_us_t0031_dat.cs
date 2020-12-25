using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0031_dat : FileMod {
		public string GetDescription() {
			return "Fix typos in Stella Garden.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x66a22b79517c7214ul, 0xb00b2a7a4ac898bcul, 0x5f231fd8u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// double spaces in various lines
			patcher.RemovePartialCommand(0xb5be, 0x3f, 0xb5e2, 1);
			patcher.RemovePartialCommand(0xeb28, 0x49, 0xeb39, 1);
			patcher.RemovePartialCommand(0xeef6, 0xc0, 0xef20, 1);

			// broken alisa line (missing space/linebreak)
			bin.Position = 0x1f7a;
			bin.WriteUInt8(0x01);
			patcher.ExtendPartialCommand(0x1f5c, 0x3c, 0x1f83, new byte[] { 0x20 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0031.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x66a22b79517c7214ul, 0xb00b2a7a4ac898bcul, 0x5f231fd8u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0031.dat", s) };
		}
	}
}
