using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib {
	public class VoiceTable {
		public List<VoiceTableEntry> Entries;
		public byte[] UnknownStartOfFileData;

		public VoiceTable(Stream s) {
			s.Position = 0;
			EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian;
			ushort count = s.ReadUInt16(e);

			if (s.PeekUInt32(e) == 1) {
				// CS2 has something odd I don't fully understand at the start of the file, try to skip past that
				UnknownStartOfFileData = s.ReadUInt8Array(0xe);
			} else {
				UnknownStartOfFileData = null;
			}

			Entries = new List<VoiceTableEntry>();
			for (int i = 0; i < count; ++i) {
				Entries.Add(new VoiceTableEntry(s, e));
			}
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteUInt16((ushort)Entries.Count, e);
			if (UnknownStartOfFileData != null) {
				s.Write(UnknownStartOfFileData);
			}
			for (int i = 0; i < Entries.Count; ++i) {
				Entries[i].WriteToStream(s, e);
			}
		}
	}

	public class VoiceTableEntry {
		public ushort Index;
		public string Name;
		public ulong Unknown1;
		public ulong Unknown2;

		public VoiceTableEntry(VoiceTableEntry other) {
			Index = other.Index;
			Name = other.Name;
			Unknown1 = other.Unknown1;
			Unknown2 = other.Unknown2;
		}

		public VoiceTableEntry(Stream s, EndianUtils.Endianness e) {
			ulong magic = s.ReadUInt48(EndianUtils.Endianness.BigEndian);
			if (magic != 0x766f69636500) {
				throw new Exception("unexpected format");
			}

			ushort entrysize = s.ReadUInt16(e);
			Index = s.ReadUInt16(e);
			Name = s.ReadAscii(entrysize - 0x11);
			Unknown1 = s.ReadUInt56(EndianUtils.Endianness.BigEndian);
			Unknown2 = s.ReadUInt64(EndianUtils.Endianness.BigEndian);

			if (Unknown1 != 0 || Unknown2 != 0x1000000c842) {
				throw new Exception("unexpected format");
			}

			return;
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteUInt48(0x766f69636500, EndianUtils.Endianness.BigEndian);
			s.WriteUInt16((ushort)(Name.Length + 0x11), e);
			s.WriteUInt16(Index, e);
			s.WriteAscii(Name);
			s.WriteUInt56(Unknown1, EndianUtils.Endianness.BigEndian);
			s.WriteUInt64(Unknown2, EndianUtils.Endianness.BigEndian);
		}

		public override string ToString() {
			return string.Format("{0}: {1} [{2:x14}{3:x16}]", Index, Name, Unknown1, Unknown2);
		}
	}
}
