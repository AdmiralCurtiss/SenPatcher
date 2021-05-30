using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_book_dat_us_book03_dat : FileMod {
		public string GetDescription() {
			return "Minor text fixes in Thors library books.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9ed835634edf89fful, 0xf1757d4fa22e4a15ul, 0x06ca0f04u));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// misplaced hyphen
			book.Entries[5].Text = book.Entries[5].Text.ReplaceSubstring(279, 1, " ", 0, 1);

			// missing word and a bunch of reformatting after that
			book.Entries[34].Text = book.Entries[34].Text.InsertSubstring(405, book.Entries[34].Text, 439, 3);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(444, 1, "\\n", 0, 2);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(495, 1, "\\n", 0, 2);
			book.Entries[34].Text = book.Entries[34].Text.ReplaceSubstring(451, 2, " ", 0, 1);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(0, 0, " ", 0, 1);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(0, 0, book.Entries[34].Text, 496, 9);
			book.Entries[34].Text = book.Entries[34].Text.Substring(0, 496);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(51, 1, "\\n", 0, 2);

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9ed835634edf89fful, 0xf1757d4fa22e4a15ul, 0x06ca0f04u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book03.dat", s) };
		}
	}
}
