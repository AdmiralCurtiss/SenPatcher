using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_heinrich_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in conversation with Heinrich.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x491c365d592bb900ul, 0x29e7543d893d47bdul, 0x5e66139du));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			bin.Position = 0x1168;
			bin.WriteUInt24(0x2e2e2e);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_heinrich.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x491c365d592bb900ul, 0x29e7543d893d47bdul, 0x5e66139du));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_heinrich.dat", s) };
		}
	}
}
