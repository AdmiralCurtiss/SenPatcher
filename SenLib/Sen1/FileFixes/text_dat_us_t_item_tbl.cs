using HyoutaUtils;
using HyoutaUtils.Streams;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	internal class ItemData {
		public ushort idx;
		public ushort unknown0;
		public string flags;
		public ushort[] unknown1;
		public string name;
		public string desc;

		public static ushort[] ReadUInt16Array(Stream s, long count, EndianUtils.Endianness endianness = EndianUtils.Endianness.LittleEndian) {
			ushort[] data = new ushort[count];
			for (long i = 0; i < count; ++i) {
				data[i] = s.ReadUInt16(endianness);
			}
			return data;
		}

		public static void WriteUInt16Array(Stream s, ushort[] data, EndianUtils.Endianness endianness = EndianUtils.Endianness.LittleEndian) {
			foreach (ushort d in data) {
				s.WriteUInt16(d, endianness);
			}
		}

		internal ItemData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			idx = stream.ReadUInt16();
			unknown0 = stream.ReadUInt16();
			flags = stream.ReadUTF8Nullterm();
			unknown1 = ReadUInt16Array(stream, 23);
			name = stream.ReadUTF8Nullterm();
			desc = stream.ReadUTF8Nullterm();
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(idx);
			ms.WriteUInt16(unknown0);
			ms.WriteUTF8Nullterm(flags);
			WriteUInt16Array(ms, unknown1);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(desc);
			return ms.CopyToByteArrayAndDispose();
		}

		public override string ToString() {
			return name + " / " + desc;
		}
	}

	public class text_dat_us_t_item_tbl : FileMod {
		public string GetDescription() {
			return "Minor fixes in item descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb64ec4d8b6204216ul, 0x6e97e60c57555203ul, 0x9c49c465u));
			if (s == null) {
				return null;
			}
			var tbl = new Tbl(s, EndianUtils.Endianness.LittleEndian);

			//List<ItemData> items = new List<ItemData>();
			//foreach (TblEntry entry in tbl.Entries) {
			//	items.Add(new ItemData(entry.Data));
			//}

			{
				int idx = 741;
				var item = new ItemData(tbl.Entries[idx].Data);
				item.desc = item.desc.Insert(5, "(R)");
				tbl.Entries[idx].Data = item.ToBinary();
			}

			MemoryStream ms = new MemoryStream();
			tbl.WriteToStream(ms, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_item.tbl", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb64ec4d8b6204216ul, 0x6e97e60c57555203ul, 0x9c49c465u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_us/t_item.tbl", s) };
		}
	}
}
