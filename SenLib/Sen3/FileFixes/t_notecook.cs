using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class CookData {
		public string name;
		public byte[] d1;
		public ushort item1;
		public string item1line1;
		public string item1line2;
		public ushort item2;
		public string item2line1;
		public string item2line2;
		public ushort item3;
		public string item3line1;
		public string item3line2;
		public ushort item4;
		public string item4line1;
		public string item4line2;
		public byte[] d2;

		internal CookData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			name = stream.ReadUTF8Nullterm();
			d1 = stream.ReadBytes(0x22);
			item1 = stream.ReadUInt16();
			item1line1 = stream.ReadUTF8Nullterm();
			item1line2 = stream.ReadUTF8Nullterm();
			item2 = stream.ReadUInt16();
			item2line1 = stream.ReadUTF8Nullterm();
			item2line2 = stream.ReadUTF8Nullterm();
			item3 = stream.ReadUInt16();
			item3line1 = stream.ReadUTF8Nullterm();
			item3line2 = stream.ReadUTF8Nullterm();
			item4 = stream.ReadUInt16();
			item4line1 = stream.ReadUTF8Nullterm();
			item4line2 = stream.ReadUTF8Nullterm();
			d2 = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUTF8Nullterm(name);
			ms.Write(d1);
			ms.WriteUInt16(item1);
			ms.WriteUTF8Nullterm(item1line1);
			ms.WriteUTF8Nullterm(item1line2);
			ms.WriteUInt16(item2);
			ms.WriteUTF8Nullterm(item2line1);
			ms.WriteUTF8Nullterm(item2line2);
			ms.WriteUInt16(item3);
			ms.WriteUTF8Nullterm(item3line1);
			ms.WriteUTF8Nullterm(item3line2);
			ms.WriteUInt16(item4);
			ms.WriteUTF8Nullterm(item4line1);
			ms.WriteUTF8Nullterm(item4line2);
			ms.Write(d2);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_notecook : FileMod {
		public string GetDescription() {
			return "Fix incorrect recipe descriptions.";
		}

		private static ItemData FindItem(ushort itemIdx, Tbl tbl) {
			// not very efficient but it's not called often enough for this to probably matter at the moment...
			for (int i = 0; i < tbl.Entries.Count; ++i) {
				if (tbl.Entries[i].Name == "item") {
					var item = new ItemData(tbl.Entries[i].Data);
					if (item.idx == itemIdx) {
						return item;
					}
				}
			}
			return null;
		}
		private static ItemHelpData FindItemHelp(ushort helpIdx, Tbl tbl) {
			// not very efficient but it's not called often enough for this to probably matter at the moment...
			for (int i = 0; i < tbl.Entries.Count; ++i) {
				if (tbl.Entries[i].Name == "ItemHelpData") {
					var item = new ItemHelpData(tbl.Entries[i].Data);
					if (item.idx == helpIdx) {
						return item;
					}
				}
			}
			return null;
		}
		private static string GenerateAttackItemLine1(ItemData item, Tbl tbl_text_en, Tbl tbl_itemhelp_en) {
			// tbqh I'm not sure how all these connections work and if the fields are right, but this works out for the given items anyway
			ushort area = item.effect1[0];
			ushort power = item.effect2[1];
			string area_string = new CompHelpData(tbl_itemhelp_en.Entries[area < 1000 ? 182 : 184].Data).str;
			string power_string = new CompHelpData(tbl_itemhelp_en.Entries[power <= 130 ? 204 : (power <= 150 ? 206 : 208)].Data).str;
			string a = new TextTableData(tbl_text_en.Entries[205].Data).str.Substring(0, 7);
			string b = new TextTableData(tbl_text_en.Entries[220].Data).str.Replace("%s", area_string);
			return a + "(Power " + power_string + ") - " + b;
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9155e5158bb3e333ul, 0x654dd761072a04e0ul, 0xa8be6728u));
			var file_text_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa2720e94f597640dul, 0xecd1d978b6b8f731ul, 0x147578a6u));
			var file_item_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x5deee9b833b2bb93ul, 0xb0a326f586943f3dul, 0x2e2424b9u));
			var file_itemhelp_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xcb9135407b8264acul, 0x813e921329374a84ul, 0x4f55036bu));
			if (file_en == null || file_text_en == null || file_item_en == null || file_itemhelp_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);
			var tbl_text_en = new Tbl(file_text_en, EndianUtils.Endianness.LittleEndian);
			var tbl_item_en = new Tbl(file_item_en, EndianUtils.Endianness.LittleEndian);
			var tbl_itemhelp_en = new Tbl(file_itemhelp_en, EndianUtils.Endianness.LittleEndian);

			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	var e = tbl_en.Entries[i];
			//	if (e.Name == "QSCook") {
			//		var m = new CookData(e.Data);
			//		m.item1line1 = "";
			//		m.item1line2 = "";
			//		m.item2line1 = "";
			//		m.item2line2 = "";
			//		m.item3line1 = "";
			//		m.item3line2 = "";
			//		m.item4line1 = "";
			//		m.item4line2 = "";
			//		e.Data = m.ToBinary();
			//	}
			//}

			// Piled Onion Rings
			{
				var m = new CookData(tbl_en.Entries[2].Data);
				string[] split = m.item4line2.Split('/');
				m.item4line1 = m.item4line1 + "/" + split[0].Substring(10);
				m.item4line2 = " " + split[1];
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 = " " + effect.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[2].Data = m.ToBinary();
			}

			// Southern Punch
			{
				var m = new CookData(tbl_en.Entries[4].Data);
				m.item1line1 = m.item1line1.Replace("EP", "CP");
				m.item2line1 = m.item2line1.Replace("EP", "CP");
				m.item4line1 = m.item4line1.Replace("EP", "CP");
				tbl_en.Entries[4].Data = m.ToBinary();
			}

			// Juicy Ham Sandwich
			{
				var m = new CookData(tbl_en.Entries[6].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 = " " + effect.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[6].Data = m.ToBinary();
			}

			// Fresh Tomato Noodles
			{
				var m = new CookData(tbl_en.Entries[7].Data);
				m.item1line1 = m.item1line1.Replace("EP", "CP");
				m.item2line1 = m.item2line1.Replace("EP", "CP");
				m.item4line1 = m.item4line1.Replace("EP", "CP");
				tbl_en.Entries[7].Data = m.ToBinary();
			}

			// Dragon Fried Rice
			{
				var m = new CookData(tbl_en.Entries[8].Data);
				string[] split = m.item1line1.Split('/');
				m.item1line1 = split[0];
				m.item1line2 = " " + split[1] + "/" + m.item1line2.Substring(1);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 = " " + effect.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[8].Data = m.ToBinary();
			}

			// Honey Bagel
			{
				var m = new CookData(tbl_en.Entries[10].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect2[0], tbl_itemhelp_en);
				m.item3line1 = " ";
				m.item3line2 = " " + effect.str;
				tbl_en.Entries[10].Data = m.ToBinary();
			}

			// Thick Hamburger Steak
			{
				var m = new CookData(tbl_en.Entries[11].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect2[0], tbl_itemhelp_en);
				m.item3line1 = " ";
				m.item3line2 = " " + effect.str;
				tbl_en.Entries[11].Data = m.ToBinary();
			}

			// Colorful Bouillabaisse
			{
				var m = new CookData(tbl_en.Entries[12].Data);
				m.item1line1 = m.item1line1.Replace("EP", "CP");
				m.item2line1 = m.item2line1.Replace("EP", "CP");
				m.item3line2 = m.item3line1.Replace("EP", "CP");
				m.item3line1 = " ";
				tbl_en.Entries[12].Data = m.ToBinary();
			}

			// Fisherman's Paella
			{
				var m = new CookData(tbl_en.Entries[13].Data);
				m.item4line1 = m.item4line1.Replace("EP", "CP");
				tbl_en.Entries[13].Data = m.ToBinary();
			}

			// Heavy Meat Pie
			{
				var m = new CookData(tbl_en.Entries[15].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 = " " + effect.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[15].Data = m.ToBinary();
			}

			// Aquamarine Ice Cream
			{
				var m = new CookData(tbl_en.Entries[16].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect0 = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				var effect1 = FindItemHelp(item3.effect4[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 =
					" " + effect0.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%") +
					"/" + effect1.str.Replace("%d", item3.effect4[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[16].Data = m.ToBinary();
			}

			// Tomato Curry
			{
				var m = new CookData(tbl_en.Entries[18].Data);
				m.item1line1 = m.item1line1.Replace("EP", "CP");
				m.item2line1 = m.item2line1.Replace("EP", "CP");
				m.item3line1 = m.item3line1.Replace("EP", "CP");
				tbl_en.Entries[18].Data = m.ToBinary();
			}

			// White Velvet Shortcake
			{
				var m = new CookData(tbl_en.Entries[19].Data);
				var item4 = FindItem(m.item4, tbl_item_en);
				var effect = FindItemHelp(item4.effect2[0], tbl_itemhelp_en);
				m.item4line1 = " ";
				m.item4line2 = " " + effect.str;
				tbl_en.Entries[19].Data = m.ToBinary();
			}

			// Croquette Burger
			{
				var m = new CookData(tbl_en.Entries[20].Data);
				var item3 = FindItem(m.item3, tbl_item_en);
				var effect0 = FindItemHelp(item3.effect3[0], tbl_itemhelp_en);
				var effect1 = FindItemHelp(item3.effect4[0], tbl_itemhelp_en);
				m.item3line1 = " " + GenerateAttackItemLine1(item3, tbl_text_en, tbl_itemhelp_en);
				m.item3line2 =
					" " + effect0.str.Replace("%d", item3.effect3[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%") +
					"/" + effect1.str.Replace("%d", item3.effect4[1].ToString("D", System.Globalization.CultureInfo.InvariantCulture)).Replace("%%", "%");
				tbl_en.Entries[20].Data = m.ToBinary();
			}

			// Hearty Kebab
			{
				var m = new CookData(tbl_en.Entries[22].Data);
				m.item1line1 = m.item1line1.Replace("EP", "CP");
				m.item2line1 = m.item2line1.Replace("EP", "CP");
				m.item3line1 = m.item3line1.Replace("EP", "CP");
				m.item4line1 = m.item4line1.Replace("EP", "CP");
				tbl_en.Entries[22].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_notecook.tbl", result_en_b.Duplicate())
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9155e5158bb3e333ul, 0x654dd761072a04e0ul, 0xa8be6728u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_notecook.tbl", file_en)
			};
		}
	}
}
