using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1010_dat : FileMod {
		public string GetDescription() {
			return "Fix double space when receiving Chapter 6 Free Day quests.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x44c4abb3f8e01ddeul, 0x0e36ca1d11cd433ful, 0x37c10788u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x25f91, 0x13f, 0x2606b, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x44c4abb3f8e01ddeul, 0x0e36ca1d11cd433ful, 0x37c10788u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1010.dat", s) };
		}
	}
}
