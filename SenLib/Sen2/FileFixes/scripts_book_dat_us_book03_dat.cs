using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book03_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 in Thors library books.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xda6f79f56616cb07ul, 0x3cc7205f895b1c0dul, 0x389c0e2du));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// misplaced hyphen
			book.Entries[5].Text = book.Entries[5].Text.ReplaceSubstring(279, 1, " ", 0, 1);

			// wording sync with CS1
			book.Entries[5].Text = book.Entries[5].Text.Remove(563, 7);
			book.Entries[5].Text = book.Entries[5].Text.ReplaceSubstring(546, 4, book.Entries[1].Text, 380, 6);

			// missing word and a bunch of reformatting after that
			book.Entries[34].Text = book.Entries[34].Text.InsertSubstring(405, book.Entries[34].Text, 439, 3);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(444, 1, "\\n", 0, 2);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(495, 1, "\\n", 0, 2);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(451, 2, " ", 0, 1);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(0, 0, " ", 0, 1);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(0, 0, book.Entries[34].Text, 496, 9);
			book.Entries[34].Text = book.Entries[34].Text.Substring(0, 496);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(51, 1, "\\n", 0, 2);

			// missing start quote
			book.Entries[13].Text = book.Entries[13].Text.InsertSubstring(54, "'", 0, 1);

			// book title change in CS1
			book.Entries[37].Text = book.Entries[37].Text.ReplaceSubstring(620, 15, "Then and Now", 0, 12);

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xda6f79f56616cb07ul, 0x3cc7205f895b1c0dul, 0x389c0e2du));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", s) };
		}
	}
}
