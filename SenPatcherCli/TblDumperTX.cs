using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using SenLib.TX;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli.TX {
	public enum TblType {
		TextTableData,
		ActiveVoiceTableData,
		QSChapter,
		QSBook,
		CharacterViewerChar,
		CharacterViewerMapRoot,
		CharacterViewerMap,
		CharacterViewerTime,
		CharacterViewerCostume,
		CharacterViewerAttach,
		CharacterViewerAction,
		dlc,
		DungeonData,
		DungeonDataB,
		hkitugi_lst,
		item,
		item_q,
		ItemHelpData,
		MapJumpData,
		LinkAbList,
		LinkAbText,
		LinkLevelExp,
		magic,
		QSTitle,
		QSText,
		SMG01HelpMessage,
		SMG02TableData,
		SMG02TableDifficulty,
		SMG02HelpMessage,
		SMG03FishPoint,
		SMG03FishPointSetting,
		SMG03Fish,
		MG04Char,
		MG04Title,
		MG04Text,
		MG04Help,
		status,
		NameTableData,
		ChangeNameData,
		NaviTextData,
		QSChar,
		QSCook,
		QSCoolVoice,
		QSCookRate,
		QSHelp,
		QSStage,
		QSMons,
		QSExDungeon,
		GridLvData,
		PlaceTableData,
		QSMaxPoint,
		QSRank,
	}

	public class TblDumper {
		public Tbl BaseTbl;

		public TblDumper(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			BaseTbl = new Tbl(stream, e, encoding);
		}

		public TblType? IdentifyEntry(int index) {
			TblType val;
			if (Enum.TryParse<TblType>(BaseTbl.Entries[index].Name, out val)) {
				return val;
			}
			Console.WriteLine("no entry for {0}", BaseTbl.Entries[index].Name);
			return null;
		}

		public static void Dump(string filenametxt, string filenametbl, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding) {
			var nf = CultureInfo.InvariantCulture.NumberFormat;
			var tbl = new TblDumper(new HyoutaUtils.Streams.DuplicatableFileStream(filenametbl), e, encoding);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < tbl.BaseTbl.Entries.Count; ++i) {
				DuplicatableByteArrayStream stream;
				TblType? tblType = tbl.IdentifyEntry(i);
				switch (tblType) {
					case TblType.TextTableData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.ActiveVoiceTableData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0}", stream.ReadUInt32(e).UIntToFloat());
						sb.AppendFormat(nf, " {0}", stream.ReadUInt32(e).UIntToFloat());
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n\n");
						break;
					}
					case TblType.QSChapter: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSBook: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSRank: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSTitle: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSText: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.PlaceTableData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSStage: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSMons: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSHelp: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSCook: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.QSChar: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.NaviTextData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.NameTableData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.ChangeNameData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.status: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.LinkAbText: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.MapJumpData: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.hkitugi_lst: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.DungeonData:
					case TblType.DungeonDataB: {
						// this is definitely wrong but good enough...
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.dlc: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, "\n{0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerChar: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerMapRoot: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerMap: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerTime: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerCostume: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x2}", stream.ReadUInt8());
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.CharacterViewerAction: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
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
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.Append("\n");
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.Append("\n");
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						if (tblType == TblType.item_q) {
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						}
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
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.magic: {
						sb.Append("[").Append(i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.Append("\n");
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
						sb.Append("\n");
						sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
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
					case TblType.SMG01HelpMessage: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.SMG02HelpMessage: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.SMG03FishPoint: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.SMG03FishPointSetting: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.SMG03Fish: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.MG04Char: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						sb.AppendFormat(nf, " {0:x8}", stream.ReadUInt32(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.MG04Text: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}
					case TblType.MG04Help: {
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
						sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
						stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
						List<string> postprint = new List<string>();
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						sb.AppendFormat(nf, " {0:x4}", stream.ReadUInt16(e));
						postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
						while (true) {
							int b = stream.ReadByte();
							if (b == -1)
								break;
							sb.AppendFormat(nf, " {0:x2}", b);
						}
						foreach (string s in postprint) {
							sb.AppendFormat(nf, " {0}", s);
						}
						sb.Append("\n");
						break;
					}

					case TblType.SMG02TableData:
					case TblType.SMG02TableDifficulty:
					case TblType.MG04Title:
					case TblType.CharacterViewerAttach:
					case TblType.LinkAbList:
					case TblType.LinkLevelExp:
					case TblType.QSExDungeon:
					case TblType.QSCoolVoice:
					case TblType.QSCookRate:
					case TblType.GridLvData:
					case TblType.QSMaxPoint:
					default:
						sb.Append("[").AppendFormat(nf, "{0:d5}", i).Append("] ");
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

		public static void AdaptSwitchFilesForPC(string path_switch, string path_pc) {
			var t_dlc_pc = new Tbl(new DuplicatableFileStream(Path.Combine(path_pc, "t_dlc.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var t_dlc_sw = new Tbl(new DuplicatableFileStream(Path.Combine(path_switch, "t_dlc.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var t_notehelp_pc = new Tbl(new DuplicatableFileStream(Path.Combine(path_pc, "t_notehelp.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var t_notehelp_sw = new Tbl(new DuplicatableFileStream(Path.Combine(path_switch, "t_notehelp.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var t_text_pc = new Tbl(new DuplicatableFileStream(Path.Combine(path_pc, "t_text.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var t_text_sw = new Tbl(new DuplicatableFileStream(Path.Combine(path_switch, "t_text.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);


			t_text_sw.Entries.RemoveAt(75);
			t_text_sw.Entries.RemoveAt(417);
			t_text_sw.Entries.RemoveAt(558);
			t_text_sw.Entries.RemoveAt(558);
			t_text_sw.Entries.RemoveAt(558);
			t_text_sw.Entries.RemoveAt(759);
			for (int i = 0; i < t_text_sw.Entries.Count; ++i) {
				var m = new TextTableData(t_text_sw.Entries[i].Data);
				if (m.idx > 0x64) {
					m.idx -= 1;
				}
				if (m.idx > 0x229) {
					m.idx -= 1;
				}
				if (m.idx > 0x264) {
					m.idx -= 3;
				}
				if (m.idx > 0x348) {
					m.idx -= 1;
				}
				t_text_sw.Entries[i].Data = m.ToBinary();
			}
			t_text_sw.Entries.Add(t_text_pc.Entries[761]);
			t_text_sw.Entries.Add(t_text_pc.Entries[762]);


			t_notehelp_sw.Entries.RemoveAt(4);
			t_notehelp_sw.Entries.RemoveAt(17);


			List<TblEntry> dlcEntries = new List<TblEntry>();
			dlcEntries.AddRange(t_dlc_sw.Entries.Take(39));
			dlcEntries.AddRange(t_dlc_pc.Entries.Skip(39).Take(4));
			dlcEntries.AddRange(t_dlc_sw.Entries.Skip(39).Take(3));
			dlcEntries.AddRange(t_dlc_pc.Entries.Skip(46).Take(3));
			dlcEntries.AddRange(t_dlc_sw.Entries.Skip(42));
			t_dlc_sw.Entries = dlcEntries;


			using (FileStream fs = new FileStream(Path.Combine(path_pc, "t_dlc.tbl.mod"), FileMode.Create)) {
				t_dlc_pc.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
			using (FileStream fs = new FileStream(Path.Combine(path_switch, "t_dlc.tbl.mod"), FileMode.Create)) {
				t_dlc_sw.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
			using (FileStream fs = new FileStream(Path.Combine(path_pc, "t_notehelp.tbl.mod"), FileMode.Create)) {
				t_notehelp_pc.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
			using (FileStream fs = new FileStream(Path.Combine(path_switch, "t_notehelp.tbl.mod"), FileMode.Create)) {
				t_notehelp_sw.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
			using (FileStream fs = new FileStream(Path.Combine(path_pc, "t_text.tbl.mod"), FileMode.Create)) {
				t_text_pc.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
			using (FileStream fs = new FileStream(Path.Combine(path_switch, "t_text.tbl.mod"), FileMode.Create)) {
				t_text_sw.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}



			var t_main_sw = new Tbl(new DuplicatableFileStream(Path.Combine(path_switch, "t_main.tbl")).CopyToByteArrayStreamAndDispose(), EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			var q = new QSText(t_main_sw.Entries[14].Data);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < 1000; ++i) {
				if (i % 10 == 0) {
					sb.Append(i);
					sb.Append(" -> ");
				}
				sb.Append("#");
				sb.Append(i);
				sb.Append("I ");
				if (i % 10 == 9) {
					sb.Append("\n");
				}
			}
			q.str = sb.ToString();
			t_main_sw.Entries[14].Data = q.ToBinary();
			using (FileStream fs = new FileStream(Path.Combine(path_switch, "t_main.tbl.mod"), FileMode.Create)) {
				t_main_sw.WriteToStream(fs, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.UTF8);
			}
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

			// affinity shards: 0x440e0

			using (var fs = new FileStream(savefilename, FileMode.Open, FileAccess.ReadWrite)) {
				fs.Position = 0x2ab84;
				bool skipMasterQuartz = true;
				for (int j = 0; j < 3; ++j) {
					int idx = 0;
					foreach (ushort itemId in itemIds) {
						if (skipMasterQuartz && itemId >= 0xc80 && itemId <= 0xc8e) {
							continue;
						}
						fs.WriteUInt32(itemId);
						fs.WriteUInt32(82);
						fs.WriteUInt32(0);
						fs.WriteUInt32(0);
						++idx;
					}
					for (; idx < 0x800; ++idx) {
						fs.WriteUInt32(9999);
						fs.WriteUInt32(0);
						fs.WriteUInt32(0);
						fs.WriteUInt32(0);
					}
				}
			}

			return;
		}
	}

	internal class TextTableData {
		public ushort idx;
		public string str;
		public byte[] d;

		internal TextTableData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			str = stream.ReadUTF8Nullterm();
			d = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUTF8Nullterm(str);
			ms.Write(d);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	internal class QSText {
		public ushort idx;
		public byte unknown;
		public string str;
		public byte[] d;

		internal QSText(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			unknown = stream.ReadUInt8();
			str = stream.ReadUTF8Nullterm();
			d = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUInt8(unknown);
			ms.WriteUTF8Nullterm(str);
			ms.Write(d);
			return ms.CopyToByteArrayAndDispose();
		}
	}
}
