using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public class VoiceDataCS3 {
		public ushort Index;
		public byte Unknown0a;
		public string Name;
		public uint Unknown0b;
		public ulong Unknown1;
		public ushort Unknown2;
		public uint Unknown3;

		public VoiceDataCS3(byte[] data, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var s = new DuplicatableByteArrayStream(data);
			Index = s.ReadUInt16(e);
			Unknown0a = s.ReadUInt8();
			Name = s.ReadNulltermString(encoding);
			Unknown0b = s.ReadUInt32(e);
			Unknown1 = s.ReadUInt64(e);
			Unknown2 = s.ReadUInt16(e);
			Unknown3 = s.ReadUInt32(e);
		}

		public byte[] ToBinary(EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var s = new System.IO.MemoryStream();
			s.WriteUInt16(Index, e);
			s.WriteUInt8(Unknown0a);
			s.WriteNulltermString(Name, encoding);
			s.WriteUInt32(Unknown0b, e);
			s.WriteUInt64(Unknown1, e);
			s.WriteUInt16(Unknown2, e);
			s.WriteUInt32(Unknown3, e);
			return s.CopyToByteArrayAndDispose();
		}
	}
}
