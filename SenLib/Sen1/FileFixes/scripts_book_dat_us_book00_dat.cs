using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_book_dat_us_book00_dat : FileMod {
		public string GetDescription() {
			return "Minor text fixes in Imperial Chronicle issues.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2eca835428184ad3ul, 0x5a9935dc5d2deaa6ul, 0x0d444aadu));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// fix inconsistent indentation
			book.Entries[43].Text = book.Entries[43].Text.Insert(209, " ");
			book.Entries[51].Text = book.Entries[51].Text.Insert(55, " ");
			book.Entries[52].Text = book.Entries[52].Text.Insert(97, " ");

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2eca835428184ad3ul, 0x5a9935dc5d2deaa6ul, 0x0d444aadu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", s) };
		}
	}
}
