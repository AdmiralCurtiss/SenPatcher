using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book04_dat : FileMod {
		public string GetDescription() {
			return "Fix translation error in headline of Imperial Chronicle Issue 3.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5b2fee612159bcb9ul, 0x3b2c6831f94f7b1ful, 0x4dd6231cu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// translation error in last headline of imperial chronicle 3
			bin.Position = 0x4124;
			bin.WriteUInt16(0x4561, EndianUtils.Endianness.BigEndian);

			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book04.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5b2fee612159bcb9ul, 0x3b2c6831f94f7b1ful, 0x4dd6231cu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book04.dat", s) };
		}
	}
}
