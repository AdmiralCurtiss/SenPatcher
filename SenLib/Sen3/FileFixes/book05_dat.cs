using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class book05_dat : FileMod {
		public string GetDescription() {
			return "Fix typos in Imperial Chronicle.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb2b35b9d531658a4ul, 0x5ed2a53477757d5aul, 0x72c066abu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var book = new BookTable(bin, EndianUtils.Endianness.LittleEndian);

			// Provincal -> Provincial
			book.Entries[3].Text = book.Entries[3].Text.Insert(514, "i");

			var bin2 = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book05.dat", bin2) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb2b35b9d531658a4ul, 0x5ed2a53477757d5aul, 0x72c066abu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/book/dat_en/book05.dat", file) };
		}
	}
}
