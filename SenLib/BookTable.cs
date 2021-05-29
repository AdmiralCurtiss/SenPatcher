using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class BookEntry {
		public string Name;
		public ushort? Book99_Value1;
		public ushort? Book99_Value2;
		public BookDataStruct[] BookDataStructs;
		public string Text;

		public override string ToString() {
			return Text;
		}
	}

	public class BookDataStruct {
		public ushort Unknown1;
		public byte[] Name;
		public ushort Unknown2;
		public ushort Unknown3;
		public ushort Unknown4;
		public ushort Unknown5;
		public ushort Unknown6;
		public ushort Unknown7;
		public ushort Unknown8;
		public ushort Unknown9;
		public ushort Unknown10;
		public ushort Unknown11;

		public BookDataStruct(Stream s, EndianUtils.Endianness e) {
			Unknown1 = s.ReadUInt16(e);
			Name = s.ReadBytes(0x10);
			Unknown2 = s.ReadUInt16(e);
			Unknown3 = s.ReadUInt16(e);
			Unknown4 = s.ReadUInt16(e);
			Unknown5 = s.ReadUInt16(e);
			Unknown6 = s.ReadUInt16(e);
			Unknown7 = s.ReadUInt16(e);
			Unknown8 = s.ReadUInt16(e);
			Unknown9 = s.ReadUInt16(e);
			Unknown10 = s.ReadUInt16(e);
			Unknown11 = s.ReadUInt16(e);
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e) {
			s.WriteUInt16(Unknown1, e);
			s.Write(Name);
			s.WriteUInt16(Unknown2, e);
			s.WriteUInt16(Unknown3, e);
			s.WriteUInt16(Unknown4, e);
			s.WriteUInt16(Unknown5, e);
			s.WriteUInt16(Unknown6, e);
			s.WriteUInt16(Unknown7, e);
			s.WriteUInt16(Unknown8, e);
			s.WriteUInt16(Unknown9, e);
			s.WriteUInt16(Unknown10, e);
			s.WriteUInt16(Unknown11, e);
		}
	}

	public class BookTable {
		public string Name;
		public uint UnknownHeaderBytes;
		public List<BookEntry> Entries;

		public BookTable(Stream s, EndianUtils.Endianness? endian = null) {
			EndianUtils.Endianness e = endian.HasValue ? endian.Value : (s.PeekUInt32(EndianUtils.Endianness.LittleEndian) == 0x20 ? EndianUtils.Endianness.LittleEndian : EndianUtils.Endianness.BigEndian);
			uint headerLength = s.ReadUInt32(e);
			if (headerLength != 0x20) {
				throw new Exception("unexpected header length");
			}

			uint namePosition = s.ReadUInt32(e);
			uint functionOffsetsPosition = s.ReadUInt32(e);
			uint functionOffsetsLength = s.ReadUInt32(e);
			uint functionNameOffsetsPosition = s.ReadUInt32(e);
			uint functionCount = s.ReadUInt32(e); // seems redundant between length and count...?
			if (functionCount * 4 != functionOffsetsLength) {
				throw new Exception("inconsistency"); // maybe one of the two means something else then?
			}
			uint functionMetadataEnd = s.ReadUInt32(e);
			UnknownHeaderBytes = s.ReadUInt32(e);

			Name = s.ReadAsciiNulltermFromLocationAndReset(namePosition);
			s.Position = functionOffsetsPosition;
			uint[] functionPositions = new uint[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				functionPositions[i] = s.ReadUInt32(e);
			}
			s.Position = functionNameOffsetsPosition;
			ushort[] functionNamePositions = new ushort[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				functionNamePositions[i] = s.ReadUInt16(e);
			}
			string[] functionNames = new string[functionCount];
			for (long i = 0; i < functionCount; ++i) {
				s.Position = functionNamePositions[i];
				functionNames[i] = s.ReadAsciiNullterm();
			}

			List<BookEntry> funcs = new List<BookEntry>();
			for (long i = 0; i < functionCount; ++i) {
				bool is99 = functionNames[i].EndsWith("_99");
				if (is99) {
					s.Position = functionPositions[i];
					ushort d1 = s.ReadUInt16(e);
					ushort d2 = s.ReadUInt16(e);
					byte terminator = s.ReadUInt8();
					if (terminator != 0x01) {
						throw new Exception("unexpected format");
					}
					funcs.Add(new BookEntry() { Name = functionNames[i], Book99_Value1 = d1, Book99_Value2 = d2 });
				} else {
					s.Position = functionPositions[i];
					ushort dataCounter = s.ReadUInt16(e);
					BookDataStruct[] data = new BookDataStruct[dataCounter];
					for (int j = 0; j < dataCounter; ++j) {
						data[j] = new BookDataStruct(s, e);
					}
					string text = s.ReadUTF8Nullterm();
					byte terminator = s.ReadUInt8();
					if (terminator != 0x01) {
						throw new Exception("unexpected format");
					}
					funcs.Add(new BookEntry() { Name = functionNames[i], BookDataStructs = data, Text = text });
				}
			}

			Entries = funcs;
		}

		public MemoryStream WriteToStream(EndianUtils.Endianness e) {
			MemoryStream s = new MemoryStream();

			// header, fill in once we know the offsets
			for (int i = 0; i < 8; ++i) {
				s.WriteUInt32(0, e);
			}

			uint namePosition = (uint)s.Position;
			s.WriteAsciiNullterm(Name);
			uint functionOffsetsPosition = (uint)s.Position;
			uint functionOffsetsLength = ((uint)Entries.Count) * 4u;
			uint functionNameOffsetsPosition = functionOffsetsPosition + functionOffsetsLength;
			uint functionCount = (uint)Entries.Count;
			uint unknown = UnknownHeaderBytes;

			// offsets to functions, fill later
			for (int i = 0; i < Entries.Count; ++i) {
				s.WriteUInt32(0, e);
			}
			// offsets to names of functions, fill later
			for (int i = 0; i < Entries.Count; ++i) {
				s.WriteUInt16(0, e);
			}

			// write function names + offsets
			for (int i = 0; i < Entries.Count; ++i) {
				long o = s.Position;
				s.WriteAsciiNullterm(Entries[i].Name);
				long p = s.Position;
				s.Position = functionOffsetsPosition + functionOffsetsLength + i * 2;
				s.WriteUInt16((ushort)o, e);
				s.Position = p;
			}

			uint functionMetadataEnd = (uint)s.Position;

			// write functions
			for (int i = 0; i < Entries.Count; ++i) {
				s.WriteAlign(4); // probably?

				long o = s.Position;

				if (Entries[i].Book99_Value1 != null) {
					s.WriteUInt16(Entries[i].Book99_Value1.Value, e);
				}
				if (Entries[i].Book99_Value2 != null) {
					s.WriteUInt16(Entries[i].Book99_Value2.Value, e);
				}
				if (Entries[i].BookDataStructs != null) {
					s.WriteUInt16((ushort)Entries[i].BookDataStructs.Length, e);
					for (int j = 0; j < Entries[i].BookDataStructs.Length; ++j) {
						Entries[i].BookDataStructs[j].WriteToStream(s, e);
					}
				}
				if (Entries[i].Text != null) {
					s.WriteUTF8Nullterm(Entries[i].Text);
				}
				s.WriteUInt8(0x01);

				long p = s.Position;
				s.Position = functionOffsetsPosition + i * 4;
				s.WriteUInt32((uint)o, e);
				s.Position = p;
			}

			// write header
			s.Position = 0;
			s.WriteUInt32(0x20, e);
			s.WriteUInt32(namePosition, e);
			s.WriteUInt32(functionOffsetsPosition, e);
			s.WriteUInt32(functionOffsetsLength, e);
			s.WriteUInt32(functionNameOffsetsPosition, e);
			s.WriteUInt32(functionCount, e);
			s.WriteUInt32(functionMetadataEnd, e);
			s.WriteUInt32(unknown, e);

			s.Position = 0;
			return s;
		}
	}
}
