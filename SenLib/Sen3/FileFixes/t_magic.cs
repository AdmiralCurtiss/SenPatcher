using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class MagicData {
		public byte[] d0;
		public string flags;
		public byte[] d1;
		public string animation;
		public string name;
		public string desc;
		public byte[] d2;

		internal MagicData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			d0 = stream.ReadBytes(4);
			flags = stream.ReadUTF8Nullterm();
			d1 = stream.ReadBytes(0x66);
			animation = stream.ReadUTF8Nullterm();
			name = stream.ReadUTF8Nullterm();
			desc = stream.ReadUTF8Nullterm();
			d2 = stream.ReadBytes(stream.Length - stream.Position);
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.Write(d0);
			ms.WriteUTF8Nullterm(flags);
			ms.Write(d1);
			ms.WriteUTF8Nullterm(animation);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(desc);
			ms.Write(d2);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_magic : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in art/craft descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc0b07b04d56268a7ul, 0xc42471d6671dc5cbul, 0x6243286fu));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	var e = tbl_en.Entries[i];
			//	if (e.Name == "magic") {
			//		var m = new MagicData(e.Data);
			//		m.flags = m.flags.Replace("Z", "");
			//		m.name = m.name.Replace("\n", " ");
			//		e.Data = m.ToBinary();
			//	}
			//}

			// a bunch of missing or extra spaces
			{
				var m = new MagicData(tbl_en.Entries[64].Data);
				m.desc = m.desc.InsertSubstring(62, " ", 0, 1);
				tbl_en.Entries[64].Data = m.ToBinary();
			}
			{
				var m = new MagicData(tbl_en.Entries[126].Data);
				m.desc = m.desc.Remove(83, 1);
				tbl_en.Entries[126].Data = m.ToBinary();
			}
			for (int i = 0; i < tbl_en.Entries.Count; ++i) {
				var e = tbl_en.Entries[i];
				if (e.Name == "magic") {
					var m = new MagicData(e.Data);
					if (m.desc.Contains(" -#")) {
						m.desc = m.desc.Replace(" -#", " - #");
						e.Data = m.ToBinary();
					}
					if (m.desc.Contains(" #0C ")) {
						m.desc = m.desc.Replace(" #0C ", "#0C ");
						e.Data = m.ToBinary();
					}
				}
			}

			// normalize newlines
			for (int i = 0; i < tbl_en.Entries.Count; ++i) {
				var e = tbl_en.Entries[i];
				if (e.Name == "magic") {
					var m = new MagicData(e.Data);
					if (m.desc.StartsWith("[") && m.flags.Contains("Z") && m.desc.Contains("\n")) {
						m.desc = t_item.AdjustNewlinesToTwoSpaces(m.desc);
						e.Data = m.ToBinary();
					}
				}
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_magic.tbl", result_en_b.Duplicate())
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xc0b07b04d56268a7ul, 0xc42471d6671dc5cbul, 0x6243286fu));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_magic.tbl", file_en)
			};
		}
	}
}
