using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
	internal class TextTableData {
		public ushort idx;
		public string str;
		public byte[] d;

		internal TextTableData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			str = stream.ReadUTF8Nullterm();
			d = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUTF8Nullterm(str);
			ms.Write(d);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_text : FileMod {
		private bool AllowSwitchToNightmare;

		public t_text(bool allowSwitchToNightmare) {
			AllowSwitchToNightmare = allowSwitchToNightmare;
		}

		public string GetDescription() {
			return "Fix a few system messages.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb80489ae03c9c909ul, 0x53e7285b920df660ul, 0x148768efu));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			//List<TextTableData> tmp = new List<TextTableData>();
			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	tmp.Add(new TextTableData(tbl_en.Entries[i].Data));
			//}

			if (AllowSwitchToNightmare) {
				var m = new TextTableData(tbl_en.Entries[110].Data);
				m.str = m.str.Split('\n')[0];
				tbl_en.Entries[110].Data = m.ToBinary();
			}

			// this description is gibberish because someone assumed %s is the MQ name instead of the character name, rewrite it so it makes sense
			for (int i = 0; i < 2; ++i) {
				var m = new TextTableData(tbl_en.Entries[144 + i].Data);
				m.str = string.Format("#1C%s has this master quartz equipped in the #3C{0}#1C slot.\nExchange?", i == 0 ? "main" : "sub");
				tbl_en.Entries[144 + i].Data = m.ToBinary();
			}

			// S-Break setting lines, these make no sense and were probably not noticed because CS3 doesn't have settable S-Breaks (but still has these lines)
			{
				var m = new TextTableData(tbl_en.Entries[123].Data);
				m.str = " will be set as S-Break.";
				tbl_en.Entries[123].Data = m.ToBinary();
			}
			{
				var m = new TextTableData(tbl_en.Entries[124].Data);
				m.str = " is already set as S-Break.";
				tbl_en.Entries[124].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_text.tbl", result_en_b.Duplicate())
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb80489ae03c9c909ul, 0x53e7285b920df660ul, 0x148768efu));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_text.tbl", file_en)
			};
		}
	}
}
