using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public class Tbl {
		public List<TblEntry> Entries;

		public Tbl(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			ushort entryCount = stream.ReadUInt16(e);

			List<TblEntry> entries = new List<TblEntry>(entryCount);
			for (int i = 0; i < entryCount; ++i) {
				var d = new TblEntry();
				d.Name = stream.ReadNulltermString(encoding);
				ushort count = GetLength(d.Name, stream, e, encoding);
				d.Data = stream.ReadBytes(count);
				entries.Add(d);
			}

			Entries = entries;
		}

		private ushort GetLength(string name, DuplicatableStream stream, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding) {
			switch (name) {
				case "item": {
						stream.ReadUInt16(e);
						long p = stream.Position;
						stream.Position += 4;
						stream.ReadNulltermString(encoding);
						stream.Position += 46;
						stream.ReadNulltermString(encoding);
						stream.ReadNulltermString(encoding);
						long l = stream.Position - p;
						stream.Position = p;
						return (ushort)l;
					}
				case "magic": {
						stream.ReadUInt16(e);
						long p = stream.Position;
						stream.Position += 4;
						stream.ReadNulltermString(encoding);
						stream.Position += 24;
						stream.ReadNulltermString(encoding);
						stream.ReadNulltermString(encoding);
						stream.ReadNulltermString(encoding);
						long l = stream.Position - p;
						stream.Position = p;
						return (ushort)l;
					}
				default:
					return stream.ReadUInt16(e);
			}
		}

		public void WriteToStream(Stream s, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			s.WriteUInt16((ushort)Entries.Count, e);
			foreach (TblEntry entry in Entries) {
				s.WriteNulltermString(entry.Name, encoding);
				s.WriteUInt16((ushort)entry.Data.Length, e);
				s.Write(entry.Data);
			}
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
