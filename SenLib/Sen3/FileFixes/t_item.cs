using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class ItemData {
		public ushort idx;
		public ushort character;
		public string flags;
		public ushort[] effect0;
		public ushort[] effect1;
		public ushort[] effect2;
		public ushort[] effect3;
		public ushort[] effect4;
		public ushort[] effect5;
		public ushort[] effect6;
		public ushort[] stats;
		public byte[] d1;
		public string name;
		public string desc;
		public byte[] d2;

		public static ushort[] ReadUInt16Array(Stream s, long count, EndianUtils.Endianness endianness = EndianUtils.Endianness.LittleEndian) {
			ushort[] data = new ushort[count];
			for (long i = 0; i < count; ++i) {
				data[i] = s.ReadUInt16(endianness);
			}
			return data;
		}

		public static void WriteUInt16Array(Stream s, ushort[] data, EndianUtils.Endianness endianness = EndianUtils.Endianness.LittleEndian) {
			foreach (ushort d in data) {
				s.WriteUInt16(d, endianness);
			}
		}

		internal ItemData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			character = stream.ReadUInt16();
			flags = stream.ReadUTF8Nullterm();
			effect0 = ReadUInt16Array(stream, 7);
			effect1 = ReadUInt16Array(stream, 7);
			effect2 = ReadUInt16Array(stream, 7);
			effect3 = ReadUInt16Array(stream, 7);
			effect4 = ReadUInt16Array(stream, 7);
			effect5 = ReadUInt16Array(stream, 7);
			effect6 = ReadUInt16Array(stream, 7);
			stats = ReadUInt16Array(stream, 10);
			d1 = stream.ReadBytes(9);
			name = stream.ReadUTF8Nullterm();
			desc = stream.ReadUTF8Nullterm();
			d2 = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUInt16(character);
			ms.WriteUTF8Nullterm(flags);
			WriteUInt16Array(ms, effect0);
			WriteUInt16Array(ms, effect1);
			WriteUInt16Array(ms, effect2);
			WriteUInt16Array(ms, effect3);
			WriteUInt16Array(ms, effect4);
			WriteUInt16Array(ms, effect5);
			WriteUInt16Array(ms, effect6);
			WriteUInt16Array(ms, stats);
			ms.Write(d1);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(desc);
			ms.Write(d2);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	internal class ItemHelpData {
		public ushort idx;
		public string str;
		public byte[] d;

		internal ItemHelpData(byte[] data) {
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

	internal class CompHelpData {
		public ushort idx;
		public string str;
		public byte[] d;

		internal CompHelpData(byte[] data) {
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

	class t_item : FileMod {
		public string GetDescription() {
			return "Fix incorrect item descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5deee9b833b2bb93ul, 0xb0a326f586943f3dul, 0x2e2424b9u));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	var e = tbl_en.Entries[i];
			//	if (e.Name == "item" || e.Name == "item_q") {
			//		var m = new ItemData(e.Data);
			//		m.flags = m.flags.Replace("Z", "");
			//		m.desc = m.desc.Replace("\n", " ");
			//		e.Data = m.ToBinary();
			//	}
			//}

			// Sennin Gem
			{
				var m = new ItemData(tbl_en.Entries[597].Data);
				m.desc = m.desc.ReplaceSubstring(36, 3, "ADF", 0, 3);
				tbl_en.Entries[597].Data = m.ToBinary();
			}

			// Seraph
			{
				var m = new ItemData(tbl_en.Entries[763].Data);
				m.desc = m.desc.Remove(65, 1);
				tbl_en.Entries[763].Data = m.ToBinary();
			}

			// Solar Gyre
			{
				var m = new ItemData(tbl_en.Entries[764].Data);
				m.desc = m.desc.ReplaceSubstring(52, 1, "/", 0, 1);
				tbl_en.Entries[764].Data = m.ToBinary();
			}

			// Impassion R
			{
				var m = new ItemData(tbl_en.Entries[619].Data);
				m.desc = m.desc.ReplaceSubstring(75, 1, "0", 0, 1);
				m.desc = m.desc.ReplaceSubstring(81, 1, "4", 0, 1);
				tbl_en.Entries[619].Data = m.ToBinary();
			}

			// Panzer Goggles
			{
				var m = new ItemData(tbl_en.Entries[303].Data);
				m.desc = m.desc.ReplaceSubstring(28, 1, "10", 0, 2);
				tbl_en.Entries[303].Data = m.ToBinary();
			}

			// Orange Corsage
			{
				var m = new ItemData(tbl_en.Entries[318].Data);
				m.desc = m.desc.Insert(18, "%");
				tbl_en.Entries[318].Data = m.ToBinary();
			}

			// Glory Emblem
			{
				var m = new ItemData(tbl_en.Entries[391].Data);
				m.desc = m.desc.Insert(12, "%");
				tbl_en.Entries[391].Data = m.ToBinary();
			}

			// Blue Star Lionheart Medal
			{
				var m = new ItemData(tbl_en.Entries[395].Data);
				string basestr = new ItemData(tbl_en.Entries[343].Data).desc.Split('\n')[0];
				m.flags = m.flags + "Z";
				m.desc = basestr.InsertSubstring(6, "STR+100/DEF+100/ATS+100/ADF+100/", 0, 32) + "\n " + m.desc;
				tbl_en.Entries[395].Data = m.ToBinary();
			}

			// Shield Potion II
			{
				var m = new ItemData(tbl_en.Entries[34].Data);
				m.desc = m.desc.ReplaceSubstring(54, 2, "30", 0, 2);
				tbl_en.Entries[34].Data = m.ToBinary();
			}

			// Energy Drink
			{
				var m = new ItemData(tbl_en.Entries[38].Data);
				m.desc = m.desc.ReplaceSubstring(39, 1, "5", 0, 1);
				m.desc = m.desc.ReplaceSubstring(58, 1, "C", 0, 1);
				tbl_en.Entries[38].Data = m.ToBinary();
			}

			// A bunch of items that claim EP when they should be CP
			foreach (int idx in new int[] {
				51, 52, 53, 54, 61, 62, 63, 64, 65, 66, 67,
				1026, 1029, 1034, 1040, 1044, 988, 991, 996, 1002,
				1006, 1015, 1019, 1022, 1048, 1051, 1057, 1066,
			}) {
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Replace("EP", "CP");
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// normalize newlines
			for (int i = 0; i < tbl_en.Entries.Count; ++i) {
				var e = tbl_en.Entries[i];
				if (e.Name == "item" || e.Name == "item_q") {
					var m = new ItemData(e.Data);
					if (m.desc.StartsWith("[") && m.flags.Contains("Z") && m.desc.Contains("\n")) {
						m.desc = AdjustNewlinesToTwoSpaces(m.desc);
						e.Data = m.ToBinary();
					}
				}
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_item_en.tbl", result_en_b.Duplicate()),
				new FileModResult("data/text/dat_fr/t_item_en.tbl", result_en_b.Duplicate())
			};
		}

		public static string AdjustNewlinesToTwoSpaces(string desc) {
			int idx = 0;
			string s = desc;
			while (true) {
				int nidx = s.IndexOf('\n', idx);
				if (nidx == -1) {
					break;
				}

				int spaces = 0;
				int i = nidx + 1;
				while (i < s.Length) {
					if (s[i] == ' ') {
						++i;
						++spaces;
					} else {
						break;
					}
				}

				if (spaces != 2) {
					if (spaces < 2) {
						s = s.InsertSubstring(nidx + 1, "  ", 0, 2 - spaces);
					} else {
						s = s.Remove(nidx + 1, spaces - 2);
					}
				}

				idx = nidx + 1;
			}
			return s;
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5deee9b833b2bb93ul, 0xb0a326f586943f3dul, 0x2e2424b9u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_item_en.tbl", file_en.Duplicate()),
				new FileModResult("data/text/dat_fr/t_item_en.tbl", file_en.Duplicate())
			};
		}
	}
}
