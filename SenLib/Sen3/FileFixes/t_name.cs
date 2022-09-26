using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class NameData {
		public ushort idx;
		public string name;
		public string str2;
		public string str3;
		public string str4;
		public string str5;
		public string str6;
		public byte[] d2;

		internal NameData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			name = stream.ReadUTF8Nullterm();
			str2 = stream.ReadUTF8Nullterm();
			str3 = stream.ReadUTF8Nullterm();
			str4 = stream.ReadUTF8Nullterm();
			str5 = stream.ReadUTF8Nullterm();
			str6 = stream.ReadUTF8Nullterm();
			d2 = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(str2);
			ms.WriteUTF8Nullterm(str3);
			ms.WriteUTF8Nullterm(str4);
			ms.WriteUTF8Nullterm(str5);
			ms.WriteUTF8Nullterm(str6);
			ms.Write(d2);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_name : FileMod {
		public string GetDescription() {
			return "Fix incorrect NPC names.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4ddf1af1f31eeb6cul, 0x9311804d88b3f779ul, 0x519df457u));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			// Tally -> Tallys (Zero/Azure/CS4 consistency)
			{
				var m = new NameData(tbl_en.Entries[595].Data);
				m.name += 's';
				tbl_en.Entries[595].Data = m.ToBinary();
			}

			// Second Lieutenant Midrun -> 2nd Lieutenant Midrun (CS4 consistency)
			{
				var m = new NameData(tbl_en.Entries[622].Data);
				m.name = m.name.ReplaceSubstring(0, 4, "2", 0, 1);
				tbl_en.Entries[622].Data = m.ToBinary();
			}

			// Second Lieutenant Sark -> 2nd Lieutenant Sark (CS4 consistency)
			{
				var m = new NameData(tbl_en.Entries[623].Data);
				m.name = m.name.ReplaceSubstring(0, 4, "2", 0, 1);
				tbl_en.Entries[623].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_name.tbl", result_en_b.Duplicate()),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4ddf1af1f31eeb6cul, 0x9311804d88b3f779ul, 0x519df457u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_name.tbl", file_en.Duplicate()),
			};
		}
	}
}
