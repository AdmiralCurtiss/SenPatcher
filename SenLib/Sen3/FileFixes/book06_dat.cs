using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class book06_dat : FileMod {
		public string GetDescription() {
			return "Fix typos in 'Other' books.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xfa7e69755a3592cdul, 0xb5196cd45ac86167ul, 0x1798384cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var book = new BookTable(bin, EndianUtils.Endianness.LittleEndian);

			// provincal -> provincial
			book.Entries[24].Text = book.Entries[24].Text.Insert(93, "i");

			var bin2 = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book06.dat", bin2) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xfa7e69755a3592cdul, 0xb5196cd45ac86167ul, 0x1798384cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book06.dat", file) };
		}
	}
}
