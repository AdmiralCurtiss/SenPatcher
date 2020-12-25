using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_c0100_dat : FileMod {
		public string GetDescription() {
			return "Fix text errors on Vainqueur Street.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9a75c8439ca12500ul, 0x437af585d0ae9474ul, 0x4b620dfdu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// fix duplicate line on Imperial Chronicle sign
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x27aed, 0x2c, 0x27b03, 0x13);

			// fix incorrect voice clips for Rean in Alisa's bike scene
			bin.Position = 0x10150;
			bin.WriteUInt16(63276, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x10253;
			bin.WriteUInt16(63278, EndianUtils.Endianness.LittleEndian);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/c0100.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9a75c8439ca12500ul, 0x437af585d0ae9474ul, 0x4b620dfdu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/c0100.dat", s) };
		}
	}
}
