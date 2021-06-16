using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public enum MstqrtTblType {
		MasterQuartzStatus,
		MasterQuartzBase,
		MasterQuartzData,
		MasterQuartzMemo,
		MasterQuartzDummy,
	}

	public class MstqrtTbl {
		public Tbl BaseTbl;

		public MstqrtTbl(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian) {
			BaseTbl = new Tbl(stream, e);
		}

		public MstqrtTblType IdentifyEntry(int index) {
			return (MstqrtTblType)Enum.Parse(typeof(MstqrtTblType), BaseTbl.Entries[index].Name);
		}

		public static void Dump(string filenametxt, string filenametbl) {
			var tbl = new MstqrtTbl(new HyoutaUtils.Streams.DuplicatableFileStream(filenametbl), EndianUtils.Endianness.LittleEndian);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < tbl.BaseTbl.Entries.Count; ++i) {
				DuplicatableByteArrayStream stream;
				switch (tbl.IdentifyEntry(i)) {
					case MstqrtTblType.MasterQuartzBase:
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" MQ {0:x4}", stream.ReadUInt16());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1) break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case MstqrtTblType.MasterQuartzData:
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" MQ {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" Lv {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" Effect1Numbers");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0,8}", stream.ReadUInt32().UIntToFloat());
						}
						sb.AppendFormat(" Effect2Numbers");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0,8}", stream.ReadUInt32().UIntToFloat());
						}
						sb.AppendFormat(" Effect3Numbers");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0,8}", stream.ReadUInt32().UIntToFloat());
						}
						sb.AppendFormat(" Unknown {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" Effect1Strings");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						}
						sb.AppendFormat(" Effect2Strings");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						}
						sb.AppendFormat(" Effect3Strings");
						for (int j = 0; j < 3; ++j) {
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						}
						while (true) {
							int b = stream.ReadByte();
							if (b == -1) break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case MstqrtTblType.MasterQuartzMemo:
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" MQ {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" String {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						break;
					default:
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						foreach (byte b in tbl.BaseTbl.Entries[i].Data) {
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
