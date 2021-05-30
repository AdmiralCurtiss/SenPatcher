using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_book_dat_us_book01_dat : FileMod {
		public string GetDescription() {
			return "Minor text fixes in Carnelia.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4243329ec1dd127cul, 0xbf68a7f68d8ce604ul, 0x2225e1ebu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// missing hyphen
			bin.Position = 0x1866;
			bin.WriteUInt8(0x2d);

			bin.Position = 0;
			var book = new BookTable(bin, EndianUtils.Endianness.LittleEndian);

			// fix the really ugly (and inconsistent with other books) position of the chapter title
			foreach (var e in book.Entries) {
				if (e.Name != null && e.Name.EndsWith("01")) {
					// this looks better but still kinda sketchy
					//e.Text = e.Text.Replace("#-570y", "#-570y#250x");
					//e.Text = e.Text.Replace("#-585y", "#-580y");

					// this matches the formatting of Red Moon Rose's titles
					// by removing the line that states the chapter number
					e.Text = RemoveLineStartingWith(e.Text, "#-570y");
					e.Text = e.Text.Replace("#-585y", "#-540y");
				}
			}

			var result = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book01.dat", result) };
		}

		private static string RemoveLineStartingWith(string text, string s) {
			int idx = text.IndexOf(s);
			int endidx = text.IndexOf("\\n", idx);
			return text.Substring(0, idx) + text.Substring(endidx + 2);
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4243329ec1dd127cul, 0xbf68a7f68d8ce604ul, 0x2225e1ebu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_us/book01.dat", s) };
		}
	}
}
