using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib {
	public class VoiceTiming {
		public List<VoiceTimingEntry> Entries;
		public string Key;
		public ushort Unknown1;
		public uint Unknown2;

		public VoiceTiming(Stream s, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			s.Position = 0;
			ushort count = s.ReadUInt16(e);
			Key = s.ReadAsciiNullterm();
			Unknown1 = s.ReadUInt16(e);
			Unknown2 = s.ReadUInt32(e);

			Entries = new List<VoiceTimingEntry>();
			for (int i = 0; i < count - 1; ++i) {
				Entries.Add(new VoiceTimingEntry(s, e));
			}
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteUInt16((ushort)(Entries.Count + 1), e);
			s.WriteAsciiNullterm(Key);
			s.WriteUInt16(Unknown1, e);
			s.WriteUInt32(Unknown2, e);
			for (int i = 0; i < Entries.Count; ++i) {
				Entries[i].WriteToStream(s, e);
			}
		}
	}

	public class VoiceTimingEntry {
		public string Key;
		public ushort Unknown1;
		public ushort Index;
		public ulong TimingData;

		public VoiceTimingEntry(Stream s, EndianUtils.Endianness e) {
			Key = s.ReadAsciiNullterm();
			Unknown1 = s.ReadUInt16(e);
			Index = s.ReadUInt16(e);
			TimingData = s.ReadUInt64(EndianUtils.Endianness.LittleEndian);
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteAsciiNullterm(Key);
			s.WriteUInt16(Unknown1, e);
			s.WriteUInt16(Index, e);
			s.WriteUInt64(TimingData, EndianUtils.Endianness.LittleEndian);
		}

		public override string ToString() {
			return string.Format("{0}: {1:x16}", Index, TimingData);
		}
	}
}
