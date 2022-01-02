using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0000_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues on Thors campus grounds.";
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

			// formatting issues in Alisa Chapter 1 Evening bonding event
			bin.SwapBytes(0x79307, 0x7930d);

			// formatting issues in Jusis Chapter 3 Day bonding event
			bin.SwapBytes(0x7c3a8, 0x7c3ab);
			bin.Position = 0x7c3cd;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7c4b5, 0x7c4bb);
			bin.SwapBytes(0x7c55a, 0x7c55f);
			bin.SwapBytes(0x7c585, 0x7c58a);
			bin.SwapBytes(0x7c651, 0x7c659);
			bin.SwapBytes(0x7c908, 0x7c910);
			bin.SwapBytes(0x7c957, 0x7c95d);
			bin.SwapBytes(0x7c9be, 0x7c9c2);
			bin.SwapBytes(0x7ce6f, 0x7ce74);

			// formatting issues in Alisa Chapter 5 Day bonding event
			bin.SwapBytes(0x79ad0, 0x79ad6);
			bin.SwapBytes(0x7a484, 0x7a488);
			bin.SwapBytes(0x7a4ab, 0x7a4b3);
			bin.SwapBytes(0x7a598, 0x7a59d);

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
