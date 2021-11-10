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

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_item.tbl", ms) };
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
