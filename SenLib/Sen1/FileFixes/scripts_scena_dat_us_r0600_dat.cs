using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0600_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues and text/voice mismatches in western Trista highway.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65044a35a4c042faul, 0xbc4a5a66fd23b0cdul, 0x8163dfdbu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			var patcher = new SenScriptPatcher(bin);

			// formatting issues in Jusis Chapter 6 Day bonding event
			bin.Position = 0xc171;
			bin.WriteUInt24(0x2e2e2e);
			bin.SwapBytes(0xbf07, 0xbf11);

			// formatting issues in Alisa Chapter 5 Day bonding event
			bin.SwapBytes(0xb00b, 0xb013);
			bin.SwapBytes(0xb2bb, 0xb2c0);
			bin.SwapBytes(0xb2e4, 0xb2e7);

			// 'U-Umm...' -> 'Umm...' (voice match)
			patcher.RemovePartialCommand(0x208b, 0x35, 0x2094, 0x2);

			// 'Ya' -> 'You' (voice match)
			patcher.ReplacePartialCommand(0x874c, 0x42, 0x8762, 0x1, new byte[] { 0x6f, 0x75 });

			// 'you' -> 'ya' (voice match)
			patcher.ReplacePartialCommand(0x8f0a, 0x5a, 0x8f5f, 0x2, new byte[] { 0x61 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0600.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65044a35a4c042faul, 0xbc4a5a66fd23b0cdul, 0x8163dfdbu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/r0600.dat", s) };
		}
	}
}
