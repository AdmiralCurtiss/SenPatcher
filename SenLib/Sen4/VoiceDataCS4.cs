using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public class VoiceDataCS4 {
		public ushort Index;
		public byte Unknown0a;
		public string Name;
		public uint Unknown0b;
		public ulong Unknown1;
		public ushort Unknown2;
		public uint Unknown3;
		public string Line;

		public VoiceDataCS4(byte[] data, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var s = new DuplicatableByteArrayStream(data);
			Index = s.ReadUInt16(e);
			Unknown0a = s.ReadUInt8();
			Name = s.ReadNulltermString(encoding);
			Unknown0b = s.ReadUInt32(e);
			Unknown1 = s.ReadUInt64(e);
			Unknown2 = s.ReadUInt16(e);
			Unknown3 = s.ReadUInt32(e);
			Line = s.ReadNulltermString(encoding);
		}
	}
}
