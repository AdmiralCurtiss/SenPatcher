using HyoutaUtils;
using HyoutaUtils.Streams;
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
			return "";
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

			//{
			//	int idx = 741;
			//	var item = new ItemData(tbl.Entries[idx].Data);
			//	item.desc = item.desc.Insert(5, "(R)");
			//	tbl.Entries[idx].Data = item.ToBinary();
			//}

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
