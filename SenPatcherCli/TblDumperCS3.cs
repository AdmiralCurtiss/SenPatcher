using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using SenLib.Sen3;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli.Sen3 {
	public enum TblType {
		MasterQuartzStatus,
		MasterQuartzBase,
		MasterQuartzData,
		MasterQuartzMemo,
		MasterQuartzDummy,

		item,
		item_q,

		ItemHelpData,
		CompHelpData,

		magic,
		magicbo,
		btcalc,

		QSCook,
		QSCoolVoice,

		QSChapter,

		QSMons,

		TextTableData,

		QSTitle,

		NameTableData,

		MapJumpData,

		VoiceTiming,

		voice,

		status, // t_mons.tbl
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
					case TblType.MasterQuartzBase:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" MQ {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.MasterQuartzData:
						sb.Append("[").Append(i).Append("] ");
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
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.MasterQuartzMemo:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" MQ {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" String {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						break;
					case TblType.MasterQuartzStatus:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" Lv {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:D4}", stream.ReadUInt16());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.TextTableData:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.QSCook: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						for (int j = 0; j < 8; ++j) {
							sb.AppendFormat(" ReqItem {0:x4}", stream.ReadUInt16());
							sb.AppendFormat(" Qty {0:x4}", stream.ReadUInt16());
						}
						for (int j = 0; j < 4; ++j) {
							sb.AppendFormat(" ProducedItem {0:x4}", stream.ReadUInt16());
							postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
							postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						}
						while (true) {
							// the rest is probably flags for who is good/bad at cooking this
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSChapter: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							// the rest is probably flags for who is good/bad at cooking this
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSMons: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							// the rest is probably flags for who is good/bad at cooking this
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.item:
					case TblType.item_q: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16()); // usable by who
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						// effects when used/equipped, in groups?
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						// stats when equipped?
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.Append("\n");
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						sb.AppendFormat(" {0:x2}", stream.ReadByte());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n\n");
						break;
					}
					case TblType.ItemHelpData: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CompHelpData: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.magic: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0}", stream.ReadUInt16());
						ushort tmp = stream.ReadUInt16();
						string flags = stream.ReadUTF8Nullterm().Replace("\n", "{n}");
						if (!flags.Contains("Z")) {
							sb.Append(" Autogenerated");
						}
						postprint.Add(flags);
						sb.AppendFormat("\n{0:x4}", tmp);
						for (int j = 0; j < 0x66; ++j) {
							sb.AppendFormat(" {0:x2}", stream.ReadByte());
						}
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						sb.Append("\n");
						sb.Append("\n");
						break;
					}
					case TblType.QSTitle: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.NameTableData:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.MapJumpData:
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.AppendFormat("\n{0}", stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.VoiceTiming:
						//sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						sb.AppendFormat(" Idx {0}", stream.ReadUInt16());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
					case TblType.voice: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0}", stream.ReadUInt16());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(" {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.status: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						sb.Append("\n");
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.AppendFormat(" {0}", stream.ReadUInt32().UIntToFloat());
						sb.Append("\n");
						// below is probably not correct in terms of types/sizes
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.Append("\n");
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						postprint.Add(stream.ReadUTF8Nullterm().Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(" {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat("\n{0}", s);
						}
						sb.Append("\n");
						sb.Append("\n");
						break;
					}
					default:
						sb.Append("[").Append(i).Append("] ");
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

		public static void InjectItemsIntoSaveFile(string savefilename, string itemtblfilename) {
			var tbl = new TblDumper(new HyoutaUtils.Streams.DuplicatableFileStream(itemtblfilename), EndianUtils.Endianness.LittleEndian);
			List<ushort> itemIds = new List<ushort>();
			for (int i = 0; i < tbl.BaseTbl.Entries.Count; ++i) {
				TblType? tblType = tbl.IdentifyEntry(i);
				if (tblType == TblType.item || tblType == TblType.item_q) {
					var stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
					itemIds.Add(stream.ReadUInt16());
				}
			}

			using (var fs = new FileStream(savefilename, FileMode.Open, FileAccess.ReadWrite)) {
				fs.Position = 0x365a0;
				for (int i = 0; i < 0x1800; ++i) {
					if (i < itemIds.Count) {
						fs.WriteUInt16(itemIds[i]);
						fs.WriteUInt16(95);
						fs.Position += 0x1c;
					} else {
						fs.WriteUInt16(9999);
						fs.WriteUInt16(0);
						fs.Position += 0x1c;
					}
				}
			}

			return;
		}
	}
}
