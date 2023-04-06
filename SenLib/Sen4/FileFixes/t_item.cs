using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
	internal class ItemData {
		public ushort idx;
		public ushort character;
		public string flags;
		public byte[] d1;
		public string name;
		public string desc;
		public byte[] d2;

		internal ItemData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			character = stream.ReadUInt16();
			flags = stream.ReadUTF8Nullterm();
			d1 = stream.ReadBytes(0x96);
			name = stream.ReadUTF8Nullterm();
			desc = stream.ReadUTF8Nullterm();
			d2 = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUInt16(character);
			ms.WriteUTF8Nullterm(flags);
			ms.Write(d1);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(desc);
			ms.Write(d2);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_item : FileMod {
		public string GetDescription() {
			return "Fix incorrect item descriptions and inconsistent formatting.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe16749c32d8a877cul, 0x4a6181337f7f20b3ul, 0x0e40f379u));
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

			// Tasty Potato Chowder
			{
				int idx = 1468;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.ReplaceSubstring(39, 2, "15", 0, 2); // 2000 HP -> 1500 HP
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Seraph
			{
				int idx = 1211;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				// inconsistent formatting
				m.desc = m.desc.Remove(61, 1);
				m.desc = m.desc.Remove(82, 1);
				m.desc = m.desc.ReplaceSubstring(112, 2, "/", 0, 1);
				m.desc = m.desc.Remove(127, 1);
				m.desc = m.desc.Insert(131, " ");
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Luck
			{
				int idx = 1198;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				// inconsistent formatting
				m.desc = m.desc.Remove(59, 1);
				m.desc = m.desc.Insert(63, " ");
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Hyoushou Gem
			{
				int idx = 1042;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(73, 1); // double space
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Bluster
			{
				int idx = 1118;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(97, 1); // 1 more turns -> 1 more turn
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Aeolus Gem
			{
				int idx = 1134;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(125, 1); // 1 more turns -> 1 more turn
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Cast 1
			{
				int idx = 1144;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(m.desc.Length - 1, 1); // stray ] at the end of the description
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Cast 2
			{
				int idx = 1154;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(m.desc.Length - 1, 1); // stray ] at the end of the description
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Cast 3
			{
				int idx = 1167;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Remove(m.desc.Length - 1, 1); // stray ] at the end of the description
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			// Iron Winged Lion Medal
			{
				int idx = 677;
				var m = new ItemData(tbl_en.Entries[idx].Data);
				m.desc = m.desc.Insert(25, " "); // missing space before parens
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] { new FileModResult("data/text/dat_en/t_item_en.tbl", result_en_b.Duplicate()) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe16749c32d8a877cul, 0x4a6181337f7f20b3ul, 0x0e40f379u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_en/t_item_en.tbl", file_en.Duplicate()) };
		}
	}
}
