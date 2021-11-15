using HyoutaUtils;
using HyoutaUtils.Streams;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class MagicData {
		public ushort Idx;
		public ushort Unknown0;
		public string Flags;
		public byte Unknown1a;
		public byte Unknown1b;
		public byte Unknown1c;
		public byte Unknown1d;
		public byte Unknown1e;
		public byte Range;
		public byte Effect1_Type;
		public ushort Effect1_Value1;
		public ushort Effect1_Value2;
		public byte Effect2_Type;
		public ushort Effect2_Value1;
		public ushort Effect2_Value2;
		public byte Unknown5e;
		public byte Unknown6a;
		public ushort Unknown6b;
		public byte Unbalance;
		public byte Unknown6d;
		public ushort Unknown6e;
		public string Anim;
		public string Name;
		public string Desc;

		public MagicData(byte[] data, EndianUtils.Endianness endian = EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var stream = new DuplicatableByteArrayStream(data);
			Idx = stream.ReadUInt16(endian);
			Unknown0 = stream.ReadUInt16(endian);
			Flags = stream.ReadNulltermString(encoding);
			Unknown1a = stream.ReadUInt8();
			Unknown1b = stream.ReadUInt8();
			Unknown1c = stream.ReadUInt8();
			Unknown1d = stream.ReadUInt8();
			Unknown1e = stream.ReadUInt8();
			Range = stream.ReadUInt8();
			Effect1_Type = stream.ReadUInt8();
			Effect1_Value1 = stream.ReadUInt16(endian);
			Effect1_Value2 = stream.ReadUInt16(endian);
			Effect2_Type = stream.ReadUInt8();
			Effect2_Value1 = stream.ReadUInt16(endian);
			Effect2_Value2 = stream.ReadUInt16(endian);
			Unknown5e = stream.ReadUInt8();
			Unknown6a = stream.ReadUInt8();
			Unknown6b = stream.ReadUInt16(endian);
			Unbalance = stream.ReadUInt8();
			Unknown6d = stream.ReadUInt8();
			Unknown6e = stream.ReadUInt16(endian);
			Anim = stream.ReadNulltermString(encoding);
			Name = stream.ReadNulltermString(encoding);
			Desc = stream.ReadNulltermString(encoding);
		}

		public byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(Idx);
			ms.WriteUInt16(Unknown0);
			ms.WriteUTF8Nullterm(Flags);
			ms.WriteUInt8(Unknown1a);
			ms.WriteUInt8(Unknown1b);
			ms.WriteUInt8(Unknown1c);
			ms.WriteUInt8(Unknown1d);
			ms.WriteUInt8(Unknown1e);
			ms.WriteUInt8(Range);
			ms.WriteUInt8(Effect1_Type);
			ms.WriteUInt16(Effect1_Value1);
			ms.WriteUInt16(Effect1_Value2);
			ms.WriteUInt8(Effect2_Type);
			ms.WriteUInt16(Effect2_Value1);
			ms.WriteUInt16(Effect2_Value2);
			ms.WriteUInt8(Unknown5e);
			ms.WriteUInt8(Unknown6a);
			ms.WriteUInt16(Unknown6b);
			ms.WriteUInt8(Unbalance);
			ms.WriteUInt8(Unknown6d);
			ms.WriteUInt16(Unknown6e);
			ms.WriteUTF8Nullterm(Anim);
			ms.WriteUTF8Nullterm(Name);
			ms.WriteUTF8Nullterm(Desc);
			return ms.CopyToByteArrayAndDispose();
		}

		public override string ToString() {
			return Name + " / " + Desc;
		}
	}

	public class text_dat_us_t_magic_tbl : FileMod {
		public string GetDescription() {
			return "Fix typo in Emma's S-Craft.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			// fix typo in Emma's S-Craft
			{
				var entry = tbl.Entries[100];
				byte tmp = entry.Data[0x37];
				entry.Data[0x37] = entry.Data[0x39];
				entry.Data[0x39] = tmp;
			}

			//List<MagicData> items = new List<MagicData>();
			//foreach (TblEntry entry in tbl.Entries) {
			//	items.Add(new MagicData(entry.Data));
			//}

			//var sb = new System.Text.StringBuilder();
			//foreach (TblEntry entry in tbl.Entries) {
			//	var item = new MagicData(entry.Data);
			//	if (item.Effect1_Type == 0x01 || item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70 || (item.Effect1_Type >= 0xd9 && item.Effect1_Type <= 0xdd) || item.Effect1_Type == 0xdf) {
			//		bool isMagic = item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70 || item.Effect1_Type == 0xda;
			//		sb.AppendFormat("{0} / Power {1} / {2} / Craft Class {3} / Art Class {4} / In-Game Description: {5}\n",
			//			item.Name, item.Effect1_Value1, isMagic ? "Magic" : "Physical", Sen2.FileFixes.text_dat_us_t_magic_tbl.GetPhysicalClass(item.Effect1_Value1),
			//			Sen2.FileFixes.text_dat_us_t_magic_tbl.GetMagicClass(item.Effect1_Value1), item.Desc.Replace("\n", "{n}")
			//		);
			//	} else {
			//		//sb.AppendFormat("{0} / In-Game Description: {1}\n", item.Name, item.Desc.Replace("\n", "{n}"));
			//	}
			//}
			//File.WriteAllText(@"c:\__ed8\__script-compare\magic-classes.txt", sb.ToString(), System.Text.Encoding.UTF8);

			for (int i = 7; i < 62; i++) {
				TblEntry entry = tbl.Entries[i];
				var m = new MagicData(entry.Data);
				if (m.Effect1_Type == 0x01 || m.Effect1_Type == 0x02 || m.Effect1_Type == 0x70 || (m.Effect1_Type >= 0xd9 && m.Effect1_Type <= 0xdd) || m.Effect1_Type == 0xdf) {
					int where = m.Desc.IndexOf(']');
					if (where != -1) {
						m.Desc = m.Desc.Insert(where, " - Class " + text_dat_us_t_item_tbl.GetMagicClass(m.Effect1_Value1));
						entry.Data = m.ToBinary();
					}
				}
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd5f7bf4c4c575efdul, 0x5699e8bbd4040b81ul, 0x276a7284u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", s) };
		}
	}
}
