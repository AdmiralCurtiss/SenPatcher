using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

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

			// capitalization consistency with main game script
			book.Entries[8].Text = book.Entries[8].Text.ReplaceSubstring(189, 1, "p", 0, 1);
			book.Entries[19].Text = book.Entries[19].Text.ReplaceSubstring(461, 1, "p", 0, 1);
			book.Entries[51].Text = book.Entries[51].Text.ReplaceSubstring(63, 1, "p", 0, 1);
			book.Entries[58].Text = book.Entries[58].Text.ReplaceSubstring(442, 1, "p", 0, 1);
			book.Entries[72].Text = book.Entries[72].Text.ReplaceSubstring(301, 1, "p", 0, 1);

			// weirdly formatted sub-headline
			book.Entries[3].Text = book.Entries[3].Text.ReplaceSubstring(37, 1, "\u25C6", 0, 1);
			book.Entries[3].Text = book.Entries[3].Text.InsertSubstring(5, "\u25C6", 0, 1);
			book.Entries[3].Text = book.Entries[3].Text.ReplaceSubstring(0, 5, book.Entries[2].Text, 0, 5);

			// insert linebreaks before headlines like CS2 does
			// and clean up whitespace for easier diffing
			CleanUpWhitespace(book);

			// the above misses a handful of linebreaks that are hard to autodetect, manually inject those
			InjectNewlines(book, 31, new int[] { 1, 5 });
			InjectNewlines(book, 34, new int[] { 5, 9 });
			InjectNewlines(book, 35, new int[] { 4, 7 });
			InjectNewlines(book, 36, new int[] { 4 });
			InjectNewlines(book, 37, new int[] { 6 });
			InjectNewlines(book, 63, new int[] { 8 });
			InjectNewlines(book, 64, new int[] { 6 });
			InjectNewlines(book, 65, new int[] { 5, 6 });
			InjectNewlines(book, 66, new int[] { 7 });
			InjectNewlines(book, 85, new int[] { 2 });

			PostSyncFixes(book);

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book00.dat", bin) };
		}

		public static void PostSyncFixes(BookTable book) {
			// a few final fixes so no entries overflow
			foreach (int idx in new int[] { 8, 14, 30, 39, 47, 54, 72, 81 }) {
				book.Entries[idx].Text = book.Entries[idx].Text.Replace("\\n\\n", "\\n");
			}
		}

		private void InjectNewlines(BookTable book, int index, int[] linebreaks) {
			var e = book.Entries[index];
			List<string> split = e.Text.Split(new string[] { "\\n" }, System.StringSplitOptions.None).ToList();
			foreach (int lb in linebreaks.Reverse()) {
				split.Insert(lb, "");
			}
			e.Text = string.Join("\\n", split);
		}

		public static void CleanUpWhitespace(BookTable book) {
			for (int i = 0; i < book.Entries.Count; ++i) {
				var e = book.Entries[i];
				if (e.Text != null) {
					List<string> split = e.Text.Split(new string[] { "\\n" }, System.StringSplitOptions.None).ToList();
					for (int j = 1; j < split.Count; ++j) {
						if (split[j].Contains("[") || split[j].Contains("\u25C6") || split[j].Contains("\u2501") || (split[j].EndsWith(":") && IsUpperAscii(split[j][5]))) {
							if (!IsWhitespaceOnlyForNewspaper(split[j - 1])) {
								split.Insert(j, "");
								++j;
							}
						}
					}
					for (int j = 0; j < split.Count; ++j) {
						if (IsWhitespaceOnlyForNewspaper(split[j])) {
							split[j] = "";
						} else {
							split[j] = RemoveEndOfStringWhitespace(split[j]);
						}
					}
					for (int j = split.Count - 1; j >= 0; --j) {
						if (IsWhitespaceOnlyForNewspaper(split[j])) {
							split.RemoveAt(j);
						} else {
							break;
						}
					}
					e.Text = string.Join("\\n", split);
				}
			}
		}

		public static bool IsUpperAscii(char c) {
			return c >= 'A' && c <= 'Z';
		}

		public static bool IsWhitespace(char c) {
			return c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\n' || c == '\u3000';
		}

		public static string RemoveEndOfStringWhitespace(string s) {
			if (s == null || s == "") {
				return s;
			}

			int i = s.Length - 1;
			if (IsWhitespace(s[i])) {
				do {
					--i;
				} while (IsWhitespace(s[i]));
				return s.Substring(0, i + 1);
			}
			return s;
		}

		public static bool IsWhitespaceOnlyForNewspaper(string s) {
			int startCheckIndex = s.StartsWith("#") ? 5 : 0;
			for (int i = startCheckIndex; i < s.Length; ++i) {
				char c = s[i];
				if (!IsWhitespace(c)) {
					return false;
				}
			}
			return true;
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
