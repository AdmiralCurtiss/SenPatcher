using HyoutaUtils;
using HyoutaUtils.Streams;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1 {
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
}
