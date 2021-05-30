using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_book_dat_us_book05_dat : FileMod {
		public string GetDescription() {
			return "Fix several consistency issues between encoded and decoded Black Records.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdcd1d5c83e8201c4ul, 0x2492abbe664f61eeul, 0x3a2187ffu));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// completely missing line
			// this was fixed in CS2 PS4 (but didn't propagate into CS3, oops)
			book.Entries[4].Text = book.Entries[4].Text.Insert(59, "scars on the land,\\n \\n");

			// whitespace changes so that the encoded variants match
			book.Entries[2].Text = book.Entries[2].Text.Remove(book.Entries[2].Text.Length - 3, 3);
			book.Entries[8].Text = book.Entries[8].Text.Remove(book.Entries[8].Text.Length - 2, 2);
			book.Entries[21].Text = book.Entries[21].Text.Remove(241, 1);
			book.Entries[23].Text = book.Entries[23].Text.ReplaceSubstring(98, 2, book.Entries[23].Text, 0, 1);
			book.Entries[23].Text = book.Entries[23].Text.ReplaceSubstring(103, 1, book.Entries[23].Text, 52, 2);

			// missing paragraph start
			book.Entries[21].Text = book.Entries[21].Text.InsertSubstring(428, book.Entries[21].Text, 0, 3);
			book.Entries[21].Text = book.Entries[21].Text.ReplaceSubstring(481, 1, book.Entries[21].Text, 55, 2);
			book.Entries[21].Text = book.Entries[21].Text.ReplaceSubstring(486, 2, book.Entries[21].Text, 0, 1);
			book.Entries[21].Text = book.Entries[21].Text.ReplaceSubstring(537, 1, book.Entries[21].Text, 55, 2);
			book.Entries[21].Text = book.Entries[21].Text.ReplaceSubstring(541, 2, book.Entries[21].Text, 0, 1);

			// sync the encoded variants
			book.Entries[63].Text = book.Entries[63].Text.Remove(274, 4);
			book.Entries[32].Text = book.Entries[63].Text;
			book.Entries[34].Text = book.Entries[34].Text.InsertSubstring(243, book.Entries[34].Text, 243, 1);
			book.Entries[65].Text = book.Entries[34].Text;
			book.Entries[35].Text = book.Entries[35].Text.InsertSubstring(17, book.Entries[35].Text, 89, 6);
			book.Entries[35].Text = book.Entries[35].Text.InsertSubstring(29, book.Entries[35].Text, 90, 2);
			book.Entries[35].Text = book.Entries[35].Text.ReplaceSubstring(40, 1, book.Entries[35].Text, 59, 5);
			book.Entries[35].Text = book.Entries[35].Text.InsertSubstring(26, book.Entries[35].Text, 31, 5);
			book.Entries[35].Text = book.Entries[35].Text.InsertSubstring(26, book.Entries[35].Text, 36, 6);
			book.Entries[35].Text = book.Entries[35].Text.InsertSubstring(26, book.Entries[35].Text, 42, 3);
			book.Entries[66].Text = book.Entries[35].Text;
			book.Entries[39].Text = book.Entries[39].Text.Remove(239, 4);
			book.Entries[68].Text = book.Entries[68].Text.ReplaceSubstring(557, 7, book.Entries[68].Text, 207, 1);
			book.Entries[37].Text = book.Entries[68].Text;
			book.Entries[70].Text = book.Entries[39].Text;
			book.Entries[49].Text = book.Entries[49].Text.Remove(488, 1);
			book.Entries[80].Text = book.Entries[49].Text;
			book.Entries[52].Text = book.Entries[52].Text.Remove(425, 11);
			book.Entries[52].Text = book.Entries[52].Text.InsertSubstring(428, book.Entries[52].Text, 0, 3);
			book.Entries[52].Text = book.Entries[52].Text.ReplaceSubstring(481, 1, book.Entries[52].Text, 55, 2);
			book.Entries[52].Text = book.Entries[52].Text.ReplaceSubstring(486, 2, book.Entries[52].Text, 0, 1);
			book.Entries[52].Text = book.Entries[52].Text.ReplaceSubstring(537, 1, book.Entries[52].Text, 55, 2);
			book.Entries[52].Text = book.Entries[52].Text.ReplaceSubstring(541, 2, book.Entries[52].Text, 0, 1);
			book.Entries[83].Text = book.Entries[52].Text;
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(23, 1, book.Entries[23].Text, 23, 7);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(51, 13, book.Entries[23].Text, 51, 54);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(24, 6, book.Entries[85].Text, 105, 6);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(62, 7, book.Entries[85].Text, 105, 7);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(71, 3, book.Entries[85].Text, 105, 3);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(75, 4, book.Entries[85].Text, 105, 4);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(81, 3, book.Entries[85].Text, 105, 3);
			book.Entries[85].Text = book.Entries[85].Text.ReplaceSubstring(85, 7, book.Entries[85].Text, 105, 7);
			book.Entries[54].Text = book.Entries[85].Text;
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(213, 3, book.Entries[7].Text, 213, 7);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(237, 93, book.Entries[7].Text, 237, 100);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(241, 3, book.Entries[38].Text, 96, 3);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(245, 4, book.Entries[38].Text, 96, 4);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(273, 9, book.Entries[38].Text, 96, 9);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(290, 8, book.Entries[38].Text, 96, 8);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(313, 5, book.Entries[38].Text, 96, 5);
			book.Entries[38].Text = book.Entries[38].Text.ReplaceSubstring(323, 6, book.Entries[38].Text, 96, 6);
			book.Entries[69].Text = book.Entries[38].Text;

			/*
			for (int i = 32; i < 62; ++i) {
				if (!(book.Entries[i].Text == book.Entries[i + 31].Text)) {
					System.Console.WriteLine("mismatch between {0} ({1}) and {2} ({3})", book.Entries[i].Name, i, book.Entries[i + 31].Name, i + 31);
				}
			}
			for (int i = 1; i < 31; ++i) {
				string decoded = book.Entries[i].Text;
				string encoded = book.Entries[i + 31].Text;
				bool same = false;
				if (decoded == null && encoded == null) {
					same = true;
				} else if (decoded.Length == encoded.Length) {
					same = true;
					for (int j = 0; j < decoded.Length; ++j) {
						char dc = decoded[j];
						char ec = encoded[j];
						if (ec == '\x2503') {
							if (!((dc >= 'A' && dc <= 'Z') || (dc >= 'a' && dc <= 'z') || (dc >= '0' && dc <= '9') || dc == '\'' || dc == '?')) {
								same = false;
								break;
							}
						} else if (dc != ec) {
							same = false;
							break;
						}
					}
				}
				if (!same) {
					System.Console.WriteLine("mismatch between decoded {0} ({1}) and encoded {2} ({3})", book.Entries[i].Name, i, book.Entries[i + 31].Name, i + 31);
				}
			}
			//*/

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book05.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xdcd1d5c83e8201c4ul, 0x2492abbe664f61eeul, 0x3a2187ffu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book05.dat", s) };
		}
	}
}
