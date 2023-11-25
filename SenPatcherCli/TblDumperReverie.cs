using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using SenLib.Sen4;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli.Hajimari {
	public enum TblType {
		LinkAbList,
		LinkAbText,
	}

	public class TblDumper {
		public Tbl BaseTbl;

		public TblDumper(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			BaseTbl = new Tbl(stream, e);
		}

		public TblType? IdentifyEntry(int index) {
			try {
				return (TblType)Enum.Parse(typeof(TblType), BaseTbl.Entries[index].Name);
			} catch (Exception) {
				Console.WriteLine("no entry for {0}", BaseTbl.Entries[index].Name);
				return null;
			}
		}

		public static void Dump(string filenametxt, string filenametbl) {
			var tbl = new TblDumper(new HyoutaUtils.Streams.DuplicatableFileStream(filenametbl), EndianUtils.Endianness.LittleEndian);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < tbl.BaseTbl.Entries.Count; ++i) {
				DuplicatableByteArrayStream stream;
				TblType? tblType = tbl.IdentifyEntry(i);
				switch (tblType) {
					case TblType.LinkAbList:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" Character {0}", stream.ReadUInt16());
						sb.AppendFormat(" Link Level {0}", stream.ReadUInt16());
						sb.AppendFormat(" Link Ability {0}", stream.ReadUInt32());
						sb.Append("\n");
						break;
					case TblType.LinkAbText:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" Link Abililty {0}", stream.ReadUInt32());
						sb.AppendFormat(" ? {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" Name [{0}]", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat(" Description [{0}]", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
				}
			}
			System.IO.File.WriteAllText(filenametxt, sb.ToString());
		}
	}
}
