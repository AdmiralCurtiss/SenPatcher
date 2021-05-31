using HyoutaUtils;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book00_dat : FileMod {
		public string GetDescription() {
			return "Consistency with CS1 in Imperial Chronicle back-issues.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7a68747acbd132c5ul, 0x215ea9c7543c37f1ul, 0x46b39d63u));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// weirdly formatted sub-headline
			book.Entries[3].Text = book.Entries[3].Text.InsertSubstring(37, "\u25C6", 0, 1);
			book.Entries[3].Text = book.Entries[3].Text.InsertSubstring(5, "\u25C6", 0, 1);
			book.Entries[3].Text = book.Entries[3].Text.ReplaceSubstring(0, 5, book.Entries[2].Text, 0, 5);
			book.Entries[3].Text = book.Entries[3].Text.ReplaceSubstring(541, 33, book.Entries[9].Text, 415, 33);

			// clean up whitespace for easier diffing
			Sen1.FileFixes.scripts_book_dat_us_book00_dat.CleanUpWhitespace(book);

			// consistency fixes to match CS1
			book.Entries[8].Text = book.Entries[8].Text.ReplaceSubstring(398, 5, book.Entries[8].Text, 57, 5);
			book.Entries[39].Text = book.Entries[39].Text.ReplaceSubstring(313, 1, book.Entries[39].Text, 72, 2);
			book.Entries[42].Text = book.Entries[42].Text.ReplaceSubstring(439, 3, book.Entries[42].Text, 631, 2);
			book.Entries[81].Text = book.Entries[81].Text.ReplaceSubstring(355, 5, book.Entries[81].Text, 55, 5);
			book.Entries[85].Text = book.Entries[85].Text.InsertSubstring(18, book.Entries[85].Text, 94, 1);

			Sen1.FileFixes.scripts_book_dat_us_book00_dat.PostSyncFixes(book);

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7a68747acbd132c5ul, 0x215ea9c7543c37f1ul, 0x46b39d63u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", s) };
		}
	}
}
