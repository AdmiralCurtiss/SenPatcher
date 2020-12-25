using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0600_dat : FileMod {
		public string GetDescription() {
			return "Fix wrong elipsis in Chapter 6 Jusis bonding event.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x65044a35a4c042faul, 0xbc4a5a66fd23b0cdul, 0x8163dfdbu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			bin.Position = 0xc171;
			bin.WriteUInt24(0x2e2e2e);

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
