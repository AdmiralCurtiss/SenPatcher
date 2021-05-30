using HyoutaUtils;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace SenLib.Sen1.FileFixes {
	public class scripts_book_dat_us_book02_dat : FileMod {
		public string GetDescription() {
			return "Minor text fixes in Red Moon Rose.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xec509088c67eb02bul, 0x279e132894f68986ul, 0x591d41afu));
			if (s == null) {
				return null;
			}
			var book = new BookTable(s, EndianUtils.Endianness.LittleEndian);

			// fix double apostrophes
			book.Entries[73].Text = book.Entries[73].Text.Remove(497, 1);
			book.Entries[148].Text = book.Entries[148].Text.Remove(46, 1);

			// fix missing linebreak
			book.Entries[321].Text = book.Entries[321].Text.InsertSubstring(476, "\\n", 0, 2);

			// the diamond pseudo-chapter-break was clearly intended to span 3 lines instead of just 1,
			// all text is layouted as if that was the case, so let's fix that
			foreach (var e in book.Entries) {
				if (e.Text != null && e.Text.Contains("\x25C7")) {
					List<string> split = e.Text.Split(new string[] { "\\n" }, System.StringSplitOptions.None).ToList();
					for (int i = 0; i < split.Count; ++i) {
						if (split[i].Contains("\x25C7")) {
							if (i == 0) {
								split.Insert(1, "");
							} else {
								split.Insert(i + 1, "");
								split.Insert(i, "");
							}
							break;
						}
					}
					e.Text = string.Join("\\n", split);
				}
			}

			var bin = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book02.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xec509088c67eb02bul, 0x279e132894f68986ul, 0x591d41afu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book02.dat", s) };
		}
	}
}
