using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli.Sen2 {
	public enum TblType {
		item,
		item_q,
		condition,
		magic,
	}

	public class TblDumper {
		public Tbl BaseTbl;

		public TblDumper(DuplicatableStream stream, EndianUtils.Endianness e = EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8) {
			BaseTbl = new Tbl(stream, e, encoding);
		}

		public TblType? IdentifyEntry(int index) {
			try {
				return (TblType)Enum.Parse(typeof(TblType), BaseTbl.Entries[index].Name);
			} catch (Exception) {
				Console.WriteLine("no entry for {0}", BaseTbl.Entries[index].Name);
				return null;
			}
		}

		public static void Dump(string filenametxt, string filenametbl, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding) {
			var tbl = new TblDumper(new HyoutaUtils.Streams.DuplicatableFileStream(filenametbl), e, encoding);
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < tbl.BaseTbl.Entries.Count; ++i) {
				DuplicatableByteArrayStream stream;
				TblType? tblType = tbl.IdentifyEntry(i);
				switch (tblType) {
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
								if (b == -1) break;
								sb.AppendFormat(" {0:x2}", b);
							}
							foreach (string s in postprint) {
								sb.AppendFormat("\n{0}", s);
							}
							sb.Append("\n\n");
							break;
						}
					case TblType.condition: {
							sb.Append("[").Append(i).Append("] ");
							sb.Append(tbl.BaseTbl.Entries[i].Name).Append(":");
							stream = new DuplicatableByteArrayStream(tbl.BaseTbl.Entries[i].Data);
							sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							while (true) {
								int b = stream.ReadByte();
								if (b == -1) break;
								sb.AppendFormat(" {0:x2}", b);
							}
							sb.Append("\n\n");
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
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.Append("\n");
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.Append("\n");
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.AppendFormat(" {0:x8}", stream.ReadUInt32(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.Append("\n");
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							while (true) {
								int b = stream.ReadByte();
								if (b == -1) break;
								sb.AppendFormat(" {0:x2}", b);
							}
							foreach (string s in postprint) {
								sb.AppendFormat("\n{0}", s);
							}
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
				// I have no idea why there's three groups here.
				// Two are probably for the main and divertissement inventories...?
				fs.Position = 0x354ec;
				for (int i = 0; i < 0x800; ++i) {
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
				for (int i = 0; i < 0x800; ++i) {
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
				for (int i = 0; i < 0x800; ++i) {
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
