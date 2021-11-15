using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_t_item_tbl : FileMod {
		public string GetDescription() {
			return "Series consistency fixes in JP item descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a735f256c69b0b2ul, 0xcd2b663820953fb4ul, 0x9523723eu));
			var magic_s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2bb6ead070625281ul, 0x87e75724d828ab0ful, 0xc8336708u));
			if (s == null || magic_s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
			var magic_tbl = new Tbl(magic_s, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
			var magic = new Dictionary<ushort, MagicData>();
			foreach (var e in magic_tbl.Entries) {
				var m = new MagicData(e.Data, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
				magic.Add(m.Idx, m);
			}

			foreach (TblEntry entry in tbl.Entries) {
				var item = new ItemData(entry.Data, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
				if (item.ItemType == 0xaa) {
					bool isSingleArt = text_dat_us_t_item_tbl.IsSingleArt(item) && magic[item.Action2Value1_Art1].Name == item.Name;

					// series consistency: add R to rare quartzes
					if (isSingleArt && item.Action2_Rarity == 1) {
						item.Name += "\uff32";
					}

					// TODO: series consistency: add magic power class to description

					// series consistency: quartz that just boost a stat and nothing else should say Stat Boost
					if (text_dat_us_t_item_tbl.IsStatusQuartz(item)) {
						item.Desc = item.Desc.Insert(5, "\u5f37\u5316");
					}

					// series consistency: quartz that have passive effects should say Special
					if (text_dat_us_t_item_tbl.IsSpecialQuartz(item)) {
						if (item.Action2_Rarity == 0) {
							item.Desc = item.Desc.Insert(5, "\u7279\u6b8a");
						} else {
							item.Desc = item.Desc.Insert(item.Action2_Rarity == 1 ? 7 : 8, " \u7279\u6b8a");
						}
					}

					entry.Data = item.ToBinary(EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
				}
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian, TextUtils.GameTextEncoding.ShiftJIS);
			return new FileModResult[] { new FileModResult("data/text/dat/t_item.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a735f256c69b0b2ul, 0xcd2b663820953fb4ul, 0x9523723eu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat/t_item.tbl", s) };
		}
	}
}
