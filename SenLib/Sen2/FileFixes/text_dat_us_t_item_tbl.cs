using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	internal class ItemData {
		public ushort Idx;
		public ushort Unknown0;
		public string Flags;
		public byte[] Unknown1;
		public string Name;
		public string Desc;
		public byte[] Unknown2;

		internal ItemData(byte[] data, bool isQuartz) {
			var stream = new DuplicatableByteArrayStream(data);
			Idx = stream.ReadUInt16();
			Unknown0 = stream.ReadUInt16();
			Flags = stream.ReadUTF8Nullterm();
			Unknown1 = stream.ReadUInt8Array(0x3c);
			Name = stream.ReadUTF8Nullterm();
			Desc = stream.ReadUTF8Nullterm();
			Unknown2 = stream.ReadUInt8Array(isQuartz ? 0x14 : 0x8);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(Idx);
			ms.WriteUInt16(Unknown0);
			ms.WriteUTF8Nullterm(Flags);
			ms.Write(Unknown1);
			ms.WriteUTF8Nullterm(Name);
			ms.WriteUTF8Nullterm(Desc);
			ms.Write(Unknown2);
			return ms.CopyToByteArrayAndDispose();
		}

		public override string ToString() {
			return Name + " / " + Desc;
		}
	}

	public class text_dat_us_t_item_tbl : FileMod {
		public string GetDescription() {
			return "Fix minor formatting errors in item descriptions and sync art and quartz descriptions.";
		}

		public static string FixHpEpCpSpacing(string desc, int start = 2) {
			for (int i = start; i < desc.Length; ++i) {
				if ((desc[i - 2] >= '0' && desc[i - 2] <= '9') && (desc[i - 1] == 'H' || desc[i - 1] == 'E' || desc[i - 1] == 'C') && desc[i] == 'P') {
					return FixHpEpCpSpacing(desc.Insert(i - 1, " "), i + 1);
				}
			}
			return desc;
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0ab9f575af611369ul, 0x4b18c0128cf1343aul, 0xc6b48300u));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			//List<ItemData> items = new List<ItemData>();
			//foreach (TblEntry entry in tbl.Entries) {
			//	if (entry.Name == "item" || entry.Name == "item_q") {
			//		items.Add(new ItemData(entry.Data, entry.Name == "item_q"));
			//	}
			//}
			//foreach (TblEntry entry in tbl.Entries) {
			//	if (entry.Name == "item" || entry.Name == "item_q") {
			//		var item = new ItemData(entry.Data, entry.Name == "item_q");
			//		item.Flags = item.Flags.Replace("Z", "");
			//		entry.Data = item.ToBinary();
			//	}
			//}

			foreach (TblEntry entry in tbl.Entries) {
				if (entry.Name == "item" || entry.Name == "item_q") {
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = FixHpEpCpSpacing(item.Desc);
					entry.Data = item.ToBinary();
				}
			}

			{
				// clarify Cheese Curry Noodles
				var entry = tbl.Entries[999];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.InsertSubstring(22, item.Desc, 31, 10);
				entry.Data = item.ToBinary();
			}

			{
				// missing gender lock on some staves
				var m = new ItemData(tbl.Entries[327].Data, false).Desc.Substring(14, 9);
				var f = new ItemData(tbl.Entries[328].Data, false).Desc.Substring(14, 11);
				{
					var entry = tbl.Entries[116];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(21, m);
					entry.Data = item.ToBinary();
				}
				{
					var entry = tbl.Entries[117];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(29, m);
					entry.Data = item.ToBinary();
				}
				{
					var entry = tbl.Entries[119];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(31, m);
					entry.Data = item.ToBinary();
				}
				{
					var entry = tbl.Entries[121];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(21, f);
					entry.Data = item.ToBinary();
				}
				{
					var entry = tbl.Entries[122];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(29, f);
					entry.Data = item.ToBinary();
				}
				{
					var entry = tbl.Entries[124];
					var item = new ItemData(entry.Data, entry.Name == "item_q");
					item.Desc = item.Desc.Insert(31, f);
					entry.Data = item.ToBinary();
				}
			}

			{
				// wrong buff order in Crystal Dress
				var entry = tbl.Entries[294];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.InsertSubstring(22, item.Desc, 7, 7);
				item.Desc = item.Desc.Remove(7, 7);
				entry.Data = item.ToBinary();
			}

			{
				// extra space in Yaksha
				var entry = tbl.Entries[587];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.Remove(9, 1);
				entry.Data = item.ToBinary();
			}

			{
				// capitalization inconsistency in Heat Up
				var entry = tbl.Entries[597];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.ReplaceSubstring(32, 9, item.Desc, 57, 9);
				entry.Data = item.ToBinary();
			}

			{
				// capitalization inconsistency in Heat Up R
				var entry = tbl.Entries[613];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.ReplaceSubstring(35, 9, item.Desc, 69, 9);
				entry.Data = item.ToBinary();
			}

			{
				// missing turn count for buff in Dark Matter/R
				var other = (new ItemData(tbl.Entries[698].Data, tbl.Entries[698].Name == "item_q")).Desc.Substring(28, 10);
				var entry = tbl.Entries[694];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.InsertSubstring(48, other, 0, other.Length);
				entry.Data = item.ToBinary();
				var entry2 = tbl.Entries[707];
				var item2 = new ItemData(entry2.Data, entry2.Name == "item_q");
				item2.Desc = item2.Desc.InsertSubstring(51, other, 0, other.Length);
				entry2.Data = item2.ToBinary();
			}

			{
				// missing turn count for buff in Seraphic Ring/R
				var other = (new ItemData(tbl.Entries[955].Data, tbl.Entries[955].Name == "item_q")).Desc.Substring(22, 10);
				var entry = tbl.Entries[699];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				item.Desc = item.Desc.InsertSubstring(60, other, 0, other.Length);
				entry.Data = item.ToBinary();
				var entry2 = tbl.Entries[712];
				var item2 = new ItemData(entry2.Data, entry2.Name == "item_q");
				item2.Desc = item2.Desc.InsertSubstring(63, other, 0, other.Length);
				entry2.Data = item2.ToBinary();
			}

			{
				// awkward phrasing on Revolution
				var entry = tbl.Entries[545];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				string str = item.Desc.Substring(45, 7);
				item.Desc = item.Desc.Remove(45, 7).InsertSubstring(52, str, 0, str.Length);
				entry.Data = item.ToBinary();
			}

			{
				// awkward phrasing on Waterfall
				var entry = tbl.Entries[586];
				var item = new ItemData(entry.Data, entry.Name == "item_q");
				string str = item.Desc.Substring(45, 6);
				item.Desc = item.Desc.Remove(45, 6).InsertSubstring(52, str, 0, str.Length);
				entry.Data = item.ToBinary();
			}

			{
				var magicStream = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x92de0d29c0ad4a9eul, 0xa935870674976924ul, 0xd5df756du));
				if (magicStream != null) {
					var magicTbl = new Tbl(magicStream, EndianUtils.Endianness.LittleEndian);
					SyncMagicDescriptions(tbl, magicTbl);

					//var magic = new Dictionary<ushort, MagicData>();
					//foreach (var e in magicTbl.Entries) {
					//	if (e.Name == "magic") {
					//		var m = new MagicData(e.Data);
					//		magic.Add(m.Idx, m);
					//	}
					//}
					//foreach (TblEntry entry in tbl.Entries) {
					//	if (entry.Name == "item_q") {
					//		var item = new ItemData(entry.Data, true);
					//		var quartzdata = new DuplicatableByteArrayStream(item.Unknown2);
					//		quartzdata.DiscardBytes(8);
					//		ushort magic0 = quartzdata.ReadUInt16();
					//		ushort magic1 = quartzdata.ReadUInt16();
					//		ushort magic2 = quartzdata.ReadUInt16();
					//		ushort magic3 = quartzdata.ReadUInt16();
					//		ushort magic4 = quartzdata.ReadUInt16();
					//		ushort magic5 = quartzdata.ReadUInt16();
					//		if (magic0 != 0xffff && magic1 == 0xffff && magic2 == 0xffff && magic3 == 0xffff && magic4 == 0xffff && magic5 == 0xffff) {
					//			string itemdesc0 = item.Desc.Split('\n')[0].Replace("(R)", "").Replace("(SR)", "");
					//			string itemdesc1 = item.Desc.Split('\n')[1];
					//			if (itemdesc1.StartsWith("(")) {
					//				itemdesc1 = itemdesc1.Substring(itemdesc1.IndexOf(")") + 2);
					//			}
					//			string magicdesc0 = magic[magic0].Desc.Split('\n')[0];
					//			string magicdesc1 = magic[magic0].Desc.Split('\n')[1];
					//			if (itemdesc0 != magicdesc0) {
					//				Console.WriteLine();
					//				Console.WriteLine(item.Name);
					//				Console.WriteLine(itemdesc0);
					//				Console.WriteLine(magicdesc0);
					//				Console.WriteLine();
					//			}
					//			if (itemdesc1 != magicdesc1) {
					//				Console.WriteLine();
					//				Console.WriteLine(item.Name);
					//				Console.WriteLine(itemdesc1);
					//				Console.WriteLine(magicdesc1);
					//				Console.WriteLine();
					//			}
					//		}
					//	}
					//}
				}
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_item.tbl", ms) };
		}

		public static void SyncMagicDescriptions(Tbl itemTbl, Tbl magicTbl) {
			SyncDescription(itemTbl, 522, magicTbl, 18, useMagic: true); // Adamantine Shield
			SyncDescription(itemTbl, 537, magicTbl, 18, useMagic: true); // Adamantine Shield R
			SyncDescription(itemTbl, 550, magicTbl, 20, useMagic: true); // Frost Edge
			SyncDescription(itemTbl, 569, magicTbl, 20, useMagic: true); // Frost Edge R
			SyncDescription(itemTbl, 593, magicTbl, 33, useMagic: false); // Volcanic Rain
			SyncDescription(itemTbl, 595, magicTbl, 35, useMagic: true); // Purgatorial Flame
			SyncDescription(itemTbl, 611, magicTbl, 35, useMagic: true); // Purgatorial Flame R
			SyncDescription(itemTbl, 598, magicTbl, 38, useMagic: true); // Forte
			SyncDescription(itemTbl, 614, magicTbl, 38, useMagic: true); // Forte R
			SyncDescription(itemTbl, 599, magicTbl, 39, useMagic: true); // La Forte
			SyncDescription(itemTbl, 615, magicTbl, 39, useMagic: true); // La Forte R
			SyncDescription(itemTbl, 632, magicTbl, 44, useMagic: true); // Ragna Vortex
			SyncDescription(itemTbl, 648, magicTbl, 44, useMagic: true); // Ragna Vortex R
			SyncDescription(itemTbl, 663, magicTbl, 48, useMagic: true); // Soul Blur
			SyncDescription(itemTbl, 679, magicTbl, 48, useMagic: true); // Soul Blur R
			SyncDescription(itemTbl, 664, magicTbl, 49, useMagic: true); // Demonic Scythe
			SyncDescription(itemTbl, 680, magicTbl, 49, useMagic: true); // Demonic Scythe R
			SyncDescription(itemTbl, 665, magicTbl, 50, useMagic: true); // Grim Butterfly
			SyncDescription(itemTbl, 681, magicTbl, 50, useMagic: true); // Grim Butterfly R
			SyncDescription(itemTbl, 669, magicTbl, 54, useMagic: true); // Chrono Burst
			SyncDescription(itemTbl, 685, magicTbl, 54, useMagic: true); // Chrono Burst R
			SyncDescription(itemTbl, 696, magicTbl, 58, useMagic: false); // Altair Channon
			SyncDescription(itemTbl, 697, magicTbl, 59, useMagic: true); // Fortuna
			SyncDescription(itemTbl, 710, magicTbl, 59, useMagic: true); // Fortuna R
			SyncDescription(itemTbl, 726, magicTbl, 65, useMagic: true); // Claiomh Solarion
			SyncDescription(itemTbl, 739, magicTbl, 65, useMagic: true); // Claiomh Solarion R
		}

		private static void SyncDescription(Tbl itemTbl, int itemId, Tbl magicTbl, int magicId, bool useMagic) {
			var itemEntry = itemTbl.Entries[itemId];
			var item = new ItemData(itemEntry.Data, itemEntry.Name == "item_q");
			var magicEntry = magicTbl.Entries[magicId];
			var magic = new MagicData(magicEntry.Data);
			int itemDescStart = item.Desc.LastIndexOf('\n') + 1;
			int magicDescStart = magic.Desc.LastIndexOf('\n') + 1;
			if (item.Desc[itemDescStart] == '(') {
				while (item.Desc[itemDescStart] != ')') { ++itemDescStart; }
				itemDescStart += 2;
			}
			if (useMagic) {
				item.Desc = item.Desc.ReplaceSubstring(itemDescStart, item.Desc.Length - itemDescStart, magic.Desc, magicDescStart, magic.Desc.Length - magicDescStart);
				itemEntry.Data = item.ToBinary();
			} else {
				magic.Desc = magic.Desc.ReplaceSubstring(magicDescStart, magic.Desc.Length - magicDescStart, item.Desc, itemDescStart, item.Desc.Length - itemDescStart);
				magicEntry.Data = magic.ToBinary();
			}
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0ab9f575af611369ul, 0x4b18c0128cf1343aul, 0xc6b48300u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_item.tbl", s) };
		}
	}
}
