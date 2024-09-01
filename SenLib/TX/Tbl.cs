using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.TX {
	public class Tbl {
		public List<TblDefinition> Definitions;
		public List<TblEntry> Entries;

		public Tbl(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			ushort entryCount = stream.ReadUInt16(e);
			uint definitionCount = stream.ReadUInt32(e);
			List<TblDefinition> definitions = new List<TblDefinition>((int)definitionCount);
			for (uint i = 0; i < definitionCount; ++i) {
				var d = new TblDefinition();
				d.Name = stream.ReadNulltermString(encoding);
				d.Count = stream.ReadUInt32(e);
				definitions.Add(d);
			}

			List<TblEntry> entries = new List<TblEntry>(entryCount);
			for (int i = 0; i < entryCount; ++i) {
				var d = new TblEntry();
				d.Name = stream.ReadNulltermString(encoding);
				ushort count = GetLength(d.Name, stream, e);
				d.Data = stream.ReadBytes(count);
				entries.Add(d);
			}

			Definitions = definitions;
			Entries = entries;
		}

		private ushort GetLength(string name, DuplicatableStream stream, EndianUtils.Endianness e) {
			switch (name) {
				default:
					return stream.ReadUInt16(e);
			}
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			s.WriteUInt16((ushort)Entries.Count, e);
			s.WriteUInt32((uint)Definitions.Count, e);
			foreach (TblDefinition def in Definitions) {
				s.WriteNulltermString(def.Name, encoding);
				s.WriteUInt32(CountEntries(def.Name), e);
			}
			foreach (TblEntry entry in Entries) {
				s.WriteNulltermString(entry.Name, encoding);
				s.WriteUInt16((ushort)entry.Data.Length, e);
				s.Write(entry.Data);
			}
		}

		public uint CountEntries(string name) {
			uint count = 0;
			foreach (TblEntry entry in Entries) {
				if (entry.Name == name) {
					++count;
				}
			}
			return count;
		}
	}

	public class TblDefinition {
		public string Name;
		public uint Count;

		public override string ToString() {
			return Name;
		}
	}

	public class TblEntry {
		public string Name;
		public byte[] Data;

		public override string ToString() {
			return Name;
		}
	}
}
