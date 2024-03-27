using HyoutaUtils;
using HyoutaUtils.Streams;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1 {
	public class VoiceDataCS1 {
		public ushort Index;
		public string Name;
		public ulong Unknown1;
		public ushort Unknown2;
		public uint Unknown3;

		public VoiceDataCS1(byte[] data, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			var s = new DuplicatableByteArrayStream(data);
			Index = s.ReadUInt16(e);
			Name = s.ReadNulltermString(encoding);
			Unknown1 = s.ReadUInt64(e);
			Unknown2 = s.ReadUInt16(e);
			Unknown3 = s.ReadUInt32(e);
		}

		public byte[] ToBinary(EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			MemoryStream s = new MemoryStream();
			s.WriteUInt16(Index, e);
			s.WriteNulltermString(Name, encoding);
			s.WriteUInt64(Unknown1, e);
			s.WriteUInt16(Unknown2, e);
			s.WriteUInt32(Unknown3, e);
			return s.CopyToByteArrayAndDispose();
		}
	}
}
