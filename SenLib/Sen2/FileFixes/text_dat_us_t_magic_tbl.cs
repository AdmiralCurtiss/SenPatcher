using HyoutaUtils;
using HyoutaUtils.Streams;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	internal class MagicData {
		public ushort Idx;
		public ushort Unknown0;
		public string Flags;
		public byte Unknown1a;
		public byte Unknown1b;
		public byte Unknown1c;
		public byte Unknown1d;
		public byte Unknown1e;
		public byte Range;
		public byte Effect1_Type; // 01 -> physical attack, 02 -> magic attack, 0x11 -> petrify chance, there's a ton of these...
		public uint Effect1_Value1; // depends on type; for attacks this is the attack power the Class is calculated from, for stat/ailments this is the % chance
		public uint Effect1_Value2; // depends on type; for stat/ailments this is the number of turns the thing is active
		public byte Unknown3a;
		public byte Unknown3b;
		public byte Unknown3c;
		public byte Unknown3d;
		public byte Effect2_Type;
		public uint Effect2_Value1;
		public uint Effect2_Value2;
		public byte Unknown5a;
		public byte Unknown5b;
		public byte Unknown5c;
		public byte Unknown5d;
		public byte Unknown5e;
		public byte Unknown6a;
		public ushort Unknown6b;
		public byte Unbalance; // 0xff == no, otherwise % extra chance of unbalance compared to default
		public byte Unknown6d;
		public ushort Unknown6e;
		public string Anim;
		public string Name;
		public string Desc;

		internal MagicData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			Idx = stream.ReadUInt16();
			Unknown0 = stream.ReadUInt16();
			Flags = stream.ReadUTF8Nullterm();
			Unknown1a = stream.ReadUInt8();
			Unknown1b = stream.ReadUInt8();
			Unknown1c = stream.ReadUInt8();
			Unknown1d = stream.ReadUInt8();
			Unknown1e = stream.ReadUInt8();
			Range = stream.ReadUInt8();
			Effect1_Type = stream.ReadUInt8();
			Effect1_Value1 = stream.ReadUInt32();
			Effect1_Value2 = stream.ReadUInt32();
			Unknown3a = stream.ReadUInt8();
			Unknown3b = stream.ReadUInt8();
			Unknown3c = stream.ReadUInt8();
			Unknown3d = stream.ReadUInt8();
			Effect2_Type = stream.ReadUInt8();
			Effect2_Value1 = stream.ReadUInt32();
			Effect2_Value2 = stream.ReadUInt32();
			Unknown5a = stream.ReadUInt8();
			Unknown5b = stream.ReadUInt8();
			Unknown5c = stream.ReadUInt8();
			Unknown5d = stream.ReadUInt8();
			Unknown5e = stream.ReadUInt8();
			Unknown6a = stream.ReadUInt8();
			Unknown6b = stream.ReadUInt16();
			Unbalance = stream.ReadUInt8();
			Unknown6d = stream.ReadUInt8();
			Unknown6e = stream.ReadUInt16();
			Anim = stream.ReadUTF8Nullterm();
			Name = stream.ReadUTF8Nullterm();
			Desc = stream.ReadUTF8Nullterm();
		}

		internal byte[] ToBinary() {
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
			ms.WriteUInt32(Effect1_Value1);
			ms.WriteUInt32(Effect1_Value2);
			ms.WriteUInt8(Unknown3a);
			ms.WriteUInt8(Unknown3b);
			ms.WriteUInt8(Unknown3c);
			ms.WriteUInt8(Unknown3d);
			ms.WriteUInt8(Effect2_Type);
			ms.WriteUInt32(Effect2_Value1);
			ms.WriteUInt32(Effect2_Value2);
			ms.WriteUInt8(Unknown5a);
			ms.WriteUInt8(Unknown5b);
			ms.WriteUInt8(Unknown5c);
			ms.WriteUInt8(Unknown5d);
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
			return "Fix typo in Emma's S-Craft and sync art and quartz descriptions.";
		}

		public static string GetPhysicalClass(long value) {
			string c = "D";
			if (value >= 110) c = "C";
			if (value >= 115) c = "C+";
			if (value >= 120) c = "B";
			if (value >= 125) c = "B+";
			if (value >= 130) c = "A";
			if (value >= 135) c = "A+";
			if (value >= 140) c = "S";
			if (value >= 150) c = "S+";
			if (value >= 250) c = "SS";
			if (value >= 300) c = "SS+";
			if (value >= 350) c = "SSS";
			if (value >= 400) c = "SSS+";
			if (value >= 450) c = "4S";
			if (value >= 500) c = "4S+";
			return c;
		}
		public static string GetMagicClass(long value) {
			string c = "D";
			if (value >= 120) c = "C";
			if (value >= 135) c = "C+";
			if (value >= 150) c = "B";
			if (value >= 165) c = "B+";
			if (value >= 180) c = "A";
			if (value >= 195) c = "A+";
			if (value >= 210) c = "S";
			if (value >= 225) c = "S+";
			if (value >= 250) c = "SS";
			if (value >= 300) c = "SS+";
			if (value >= 350) c = "SSS";
			if (value >= 400) c = "SSS+";
			if (value >= 450) c = "4S";
			if (value >= 600) c = "5S";
			return c;
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			// fix typo in Emma's S-Craft
			{
				var entry = tbl.Entries[144];
				byte tmp = entry.Data[0x48];
				entry.Data[0x48] = entry.Data[0x4a];
				entry.Data[0x4a] = tmp;
			}

			//List<MagicData> items = new List<MagicData>();
			//foreach (TblEntry entry in tbl.Entries) {
			//	if (entry.Name == "magic") {
			//		items.Add(new MagicData(entry.Data));
			//	}
			//}
			//uint attackPowerCounter = 100;
			//foreach (TblEntry entry in tbl.Entries) {
			//	if (entry.Name == "magic") {
			//		var item = new MagicData(entry.Data);
			//		item.Flags = item.Flags.Replace("Z", "");
			//		if (item.Effect1_Type == 0x01) {
			//			item.Effect1_Value1 = attackPowerCounter;
			//			item.Desc = "Power == " + attackPowerCounter.ToString();
			//			attackPowerCounter += 10;
			//			item.Range = 6;
			//		}
			//		entry.Data = item.ToBinary();
			//	}
			//}

			//var sb = new System.Text.StringBuilder();
			//foreach (TblEntry entry in tbl.Entries) {
			//	if (entry.Name == "magic") {
			//		var item = new MagicData(entry.Data);
			//		if (item.Effect1_Type == 0x01 || item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70) {
			//			bool isMagic = item.Effect1_Type == 0x02 || item.Effect1_Type == 0x70;
			//			sb.AppendFormat("{0} / Power {1} / {2} / Craft Class {3} / Art Class {4} / In-Game Description: {5}\n",
			//				item.Name, item.Effect1_Value1, isMagic ? "Magic" : "Physical", GetPhysicalClass(item.Effect1_Value1),
			//				GetMagicClass(item.Effect1_Value1), item.Desc.Replace("\n", "{n}")
			//			);
			//		} else {
			//			//sb.AppendFormat("{0} / In-Game Description: {1}\n", item.Name, item.Desc.Replace("\n", "{n}"));
			//		}
			//	}
			//}
			//File.WriteAllText(@"c:\__ed8\__script-compare_cs2\magic-classes.txt", sb.ToString(), System.Text.Encoding.UTF8);

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_magic.tbl", s) };
		}
	}
}
