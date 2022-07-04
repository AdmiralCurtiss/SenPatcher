using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenPatcherCli {
	public class VoiceTable {
		public List<VoiceTableEntry> Entries;
		public byte[] UnknownStartOfFileData;

		public VoiceTable(Stream s, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			s.Position = 0;
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
		public byte Unknown0;
		public ulong Unknown1;
		public ushort Unknown2;
		public uint Unknown3;

		public VoiceTableEntry(VoiceTableEntry other) {
			Index = other.Index;
			Name = other.Name;
			Unknown0 = other.Unknown0;
			Unknown1 = other.Unknown1;
			Unknown2 = other.Unknown2;
			Unknown3 = other.Unknown3;
		}

		public VoiceTableEntry(Stream s, EndianUtils.Endianness e) {
			ulong magic = s.ReadUInt48(EndianUtils.Endianness.BigEndian);
			if (magic != 0x766f69636500) {
				throw new Exception("unexpected format");
			}

			ushort entrysize = s.ReadUInt16(e);
			Index = s.ReadUInt16(e);
			Name = s.ReadAscii(entrysize - 0x11);
			Unknown0 = s.ReadUInt8();
			Unknown1 = s.ReadUInt64(e);
			Unknown2 = s.ReadUInt16(e);
			Unknown3 = s.ReadUInt32(e);

			if (Unknown0 != 0 || Unknown1 != 0 || Unknown2 != 1 || Unknown3 != 0x42c80000) {
				throw new Exception("unexpected format");
			}

			return;
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteUInt48(0x766f69636500, EndianUtils.Endianness.BigEndian);
			s.WriteUInt16((ushort)(Name.Length + 0x11), e);
			s.WriteUInt16(Index, e);
			s.WriteAscii(Name);
			s.WriteUInt8(Unknown0);
			s.WriteUInt64(Unknown1, e);
			s.WriteUInt16(Unknown2, e);
			s.WriteUInt32(Unknown3, e);
		}

		public override string ToString() {
			return string.Format("{0}: {1} [{2:x2}{3:x16} {4:x4} {5:x8}]", Index, Name, Unknown0, Unknown1, Unknown2, Unknown3);
		}
	}
}
