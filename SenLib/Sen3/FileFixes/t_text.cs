using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t_text : FileMod {
		private bool AllowSwitchToNightmare;

		public t_text(bool allowSwitchToNightmare) {
			AllowSwitchToNightmare = allowSwitchToNightmare;
		}

		public string GetDescription() {
			return "Fix a few system messages.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa2720e94f597640dul, 0xecd1d978b6b8f731ul, 0x147578a6u));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			//List<TextTableData> tmp = new List<TextTableData>();
			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	tmp.Add(new TextTableData(tbl_en.Entries[i].Data));
			//}

			if (AllowSwitchToNightmare) {
				var m = new TextTableData(tbl_en.Entries[99].Data);
				m.str = m.str.Split('\n')[0];
				tbl_en.Entries[99].Data = m.ToBinary();
			}

			// this description is gibberish because someone assumed %s is the MQ name instead of the character name, rewrite it so it makes sense
			for (int i = 0; i < 2; ++i) {
				var m = new TextTableData(tbl_en.Entries[127 + i].Data);
				m.str = string.Format("#1C%s has this master quartz equipped in the #3C{0}#1C slot.\nExchange?", i == 0 ? "main" : "sub");
				tbl_en.Entries[127 + i].Data = m.ToBinary();
			}

			// S-Break setting lines, these make no sense and were probably not noticed because CS3 doesn't have settable S-Breaks (but still has these lines)
			{
				var m = new TextTableData(tbl_en.Entries[110].Data);
				m.str = " will be set as S-Break.";
				tbl_en.Entries[110].Data = m.ToBinary();
			}
			{
				var m = new TextTableData(tbl_en.Entries[111].Data);
				m.str = " is already set as S-Break.";
				tbl_en.Entries[111].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_text.tbl", result_en_b.Duplicate())
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa2720e94f597640dul, 0xecd1d978b6b8f731ul, 0x147578a6u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_text.tbl", file_en)
			};
		}
	}
}
