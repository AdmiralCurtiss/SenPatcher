using HyoutaPluginBase;
using HyoutaUtils;
using HyoutaUtils.Streams;
using SenLib;
using SenLib.Sen1;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli.Sen1 {
	public enum TblType {
		item,
		magic,
		QSCook,
	}

	public class TblDumper {
		public Tbl t_item;
		public Tbl t_magic;
		public Tbl t_notecook;
		private EndianUtils.Endianness Endian;
		private TextUtils.GameTextEncoding Encoding;

		private Dictionary<ushort, SenLib.Sen1.FileFixes.MagicData> Magic;

		public TblDumper(
			DuplicatableStream t_item_stream,
			DuplicatableStream t_magic_stream,
			DuplicatableStream t_notecook_stream,
			EndianUtils.Endianness endian = EndianUtils.Endianness.LittleEndian,
			TextUtils.GameTextEncoding encoding = TextUtils.GameTextEncoding.UTF8
		) {
			Endian = endian;
			Encoding = encoding;
			t_item = new Tbl(t_item_stream, endian, encoding);
			t_magic = new Tbl(t_magic_stream, endian, encoding);
			t_notecook = new Tbl(t_notecook_stream, endian, encoding);

			Magic = new Dictionary<ushort, SenLib.Sen1.FileFixes.MagicData>();
			foreach (var e in t_magic.Entries) {
				var m = new SenLib.Sen1.FileFixes.MagicData(e.Data, endian, encoding);
				Magic.Add(m.Idx, m);
			}
		}

		public static TblType? IdentifyEntry(Tbl tbl, int index) {
			try {
				return (TblType)Enum.Parse(typeof(TblType), tbl.Entries[index].Name);
			} catch (Exception) {
				Console.WriteLine("no entry for {0}", tbl.Entries[index].Name);
				return null;
			}
		}

		public void Dump(string path) {
			Dump(t_item, Path.Combine(path, "t_item.txt"), Endian, Encoding);
			Dump(t_magic, Path.Combine(path, "t_magic.txt"), Endian, Encoding);
			Dump(t_notecook, Path.Combine(path, "t_notecook.txt"), Endian, Encoding);
		}

		public void Dump(Tbl tbl, string filenametxt, EndianUtils.Endianness e, TextUtils.GameTextEncoding encoding) {
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < tbl.Entries.Count; ++i) {
				DuplicatableByteArrayStream stream;
				TblType? tblType = IdentifyEntry(tbl, i);
				switch (tblType) {
					case TblType.item: {
							sb.Append("[").Append(i).Append("] ");
							sb.Append(tbl.Entries[i].Name).Append(":");
							stream = new DuplicatableByteArrayStream(tbl.Entries[i].Data);
							List<string> postprint = new List<string>();
							sb.AppendFormat(" Idx {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" Usable by {0:x4}", stream.ReadUInt16(e));
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							sb.Append("\n");
							byte itemType = stream.ReadUInt8();
							sb.AppendFormat(" Type {0:x2}", itemType);
							sb.AppendFormat(" | Element {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" | Status/Breaker {0:x2}", stream.ReadUInt8()); // 1 == status quartz, 2 == breaker quartz, probably used to limit the equip-one-per-line
							sb.AppendFormat(" | RNG+{0,3}", stream.ReadUInt8());
							sb.AppendFormat(" | AttackArea {0,3}", stream.ReadUInt8()); // AoE of physical attack
							sb.Append("\n");
							if (itemType == 0xaa) {
								byte passiveEffect = stream.ReadUInt8(); // eg. status ailment or breaker stat
								ushort art2 = stream.ReadUInt16(e);
								ushort art3 = stream.ReadUInt16(e);
								byte rarity = stream.ReadUInt8(); // 0 = N, 1 == R, 2 = SR
								ushort art1 = stream.ReadUInt16(e);
								ushort procChance = stream.ReadUInt16(e); // for status/breaker
								sb.AppendFormat(" Art1 {0}", art1 == 0xffff ? "(None)" : Magic[art1].Name);
								sb.AppendFormat(" | Art2 {0}", art2 == 0xffff ? "(None)" : Magic[art2].Name);
								sb.AppendFormat(" | Art3 {0}", art3 == 0xffff ? "(None)" : Magic[art3].Name);
								sb.Append("\n");
								sb.AppendFormat(" Rarity  {0:x2}", rarity);
								sb.AppendFormat(" | Passive {0:x2}", passiveEffect);
								sb.AppendFormat(" | Chance {0,3}", procChance);
								sb.Append("\n");
							} else {
								sb.AppendFormat(" Action {0:x2}", stream.ReadUInt8());
								sb.AppendFormat(" | Value1 {0,5}", stream.ReadUInt16(e));
								sb.AppendFormat(" | Value2 {0,5}", stream.ReadUInt16(e));
								sb.Append("\n");
								sb.AppendFormat(" Action {0:x2}", stream.ReadUInt8());
								sb.AppendFormat(" | Value1 {0,5}", stream.ReadUInt16(e));
								sb.AppendFormat(" | Value2 {0,5}", stream.ReadUInt16(e));
								sb.Append("\n");
							}
							sb.AppendFormat(" STR+{0,5} ", stream.ReadUInt16(e));
							sb.AppendFormat(" | DEF+{0,5} ", stream.ReadUInt16(e));
							sb.AppendFormat(" | ATS+{0,5} ", stream.ReadUInt16(e));
							sb.AppendFormat(" | ADF+{0,5} ", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" ACC+{0,5}%", stream.ReadUInt16(e));
							sb.AppendFormat(" | EVA+{0,5}%", stream.ReadUInt16(e));
							sb.AppendFormat(" | SPD+{0,5} ", stream.ReadUInt16(e));
							sb.AppendFormat(" | MOV+{0,5} ", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" HP+ {0,5} ", stream.ReadUInt16(e));
							sb.AppendFormat(" | EP+ {0,5} ", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" Mira {0,6}", stream.ReadUInt32(e));
							sb.AppendFormat(" | CarryLimit {0,3}", stream.ReadUInt8()); // max you're allowed to have
							sb.AppendFormat(" | SortOrder {0,5}", stream.ReadUInt16(e)); // within the type
							sb.AppendFormat(" | ? {0,5}", stream.ReadUInt16(e)); // only shows up on quartzes, no idea what this is though
							sb.AppendFormat(" | ? {0:x4}", stream.ReadUInt16(e)); // DLC related? or maybe costume related?
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							sb.Append("\n");
							while (true) {
								int b = stream.ReadByte();
								if (b == -1) break;
								sb.AppendFormat(" {0:x2}", b);
							}
							foreach (string s in postprint) {
								sb.AppendFormat("\n{0}", s);
							}
							sb.Append("\n\n===========================================\n\n");
							break;
						}
					case TblType.magic: {
							sb.Append("[").Append(i).Append("] ");
							sb.Append(tbl.Entries[i].Name).Append(":");
							stream = new DuplicatableByteArrayStream(tbl.Entries[i].Data);
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
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.Append("\n");
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x2}", stream.ReadUInt8());
							sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
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
							sb.Append("\n===========================================\n");
							break;
						}
					case TblType.QSCook: {
							sb.Append("[").Append(i).Append("] ");
							sb.Append(tbl.Entries[i].Name).Append(":");
							stream = new DuplicatableByteArrayStream(tbl.Entries[i].Data);
							List<string> postprint = new List<string>();
							postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
							sb.AppendFormat("Idx {0:x4}", stream.ReadUInt16(e));
							for (int j = 0; j < 8; ++j) {
								sb.AppendFormat(" ReqItem {0:x4}", stream.ReadUInt16(e));
								sb.AppendFormat(" Qty {0:x4}", stream.ReadUInt16(e));
								sb.Append("\n");
							}
							for (int j = 0; j < 4; ++j) {
								sb.AppendFormat(" {0:x4}", stream.ReadUInt16(e));
								postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
								postprint.Add(stream.ReadNulltermString(encoding).Replace("\n", "{n}"));
								sb.Append("\n");
							}
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
						sb.Append(tbl.Entries[i].Name).Append(":");
						foreach (byte b in tbl.Entries[i].Data) {
							sb.AppendFormat(" {0:x2}", b);
						}
						sb.Append("\n");
						break;
				}
			}
			System.IO.File.WriteAllText(filenametxt, sb.ToString());
		}

		public static void InjectItemsIntoSaveFile(string savefilename, string itemtblfilename) {
			var tbl = new Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(itemtblfilename), EndianUtils.Endianness.LittleEndian);
			List<ushort> itemIds = new List<ushort>();
			for (int i = 0; i < tbl.Entries.Count; ++i) {
				if (tbl.Entries[i].Name == "item") {
					var stream = new DuplicatableByteArrayStream(tbl.Entries[i].Data);
					itemIds.Add(stream.ReadUInt16());
				}
			}

			using (var fs = new FileStream(savefilename, FileMode.Open, FileAccess.ReadWrite)) {
				fs.Position = 0x40c24;
				for (int i = 0; i < 0x1000; ++i) {
					if (i < itemIds.Count) {
						fs.WriteUInt16(itemIds[i]);
						fs.WriteUInt16(95);
						fs.Position += 0x20;
					} else {
						fs.WriteUInt16(9999);
						fs.WriteUInt16(0);
						fs.Position += 0x20;
					}
				}
			}

			return;
		}
	}
}
