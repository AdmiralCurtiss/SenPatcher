using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
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
	public class text_dat_us_t_notecook_tbl : FileMod {
		public string GetDescription() {
			return "Adjust formatting in cooking descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa3e1c05218c7059cul, 0x371f3641a1111e16ul, 0x6e899087u));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			foreach (TblEntry entry in tbl.Entries) {
				if (entry.Name == "QSCook") {
					var item = new CookData(entry.Data);
					item.item1line1 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item1line1);
					item.item1line2 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item1line2);
					item.item2line1 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item2line1);
					item.item2line2 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item2line2);
					item.item3line1 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item3line1);
					item.item3line2 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item3line2);
					item.item4line1 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item4line1);
					item.item4line2 = Sen2.FileFixes.text_dat_us_t_item_tbl.FixHpEpCpSpacing(item.item4line2);
					entry.Data = item.ToBinary();
				}
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_notecook.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xa3e1c05218c7059cul, 0x371f3641a1111e16ul, 0x6e899087u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_notecook.tbl", s) };
		}
	}
}
