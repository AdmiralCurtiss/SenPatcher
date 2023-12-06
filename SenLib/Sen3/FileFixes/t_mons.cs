using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	internal class MonsterData {
		public string file1;
		public string file2;
		public string file3;
		public byte[] d0;
		public string flags;
		public string name;
		public string desc;

		internal MonsterData(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			file1 = stream.ReadUTF8Nullterm();
			file2 = stream.ReadUTF8Nullterm();
			file3 = stream.ReadUTF8Nullterm();
			d0 = stream.ReadBytes(0xc5);
			flags = stream.ReadUTF8Nullterm();
			name = stream.ReadUTF8Nullterm();
			desc = stream.ReadUTF8Nullterm();
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUTF8Nullterm(file1);
			ms.WriteUTF8Nullterm(file2);
			ms.WriteUTF8Nullterm(file3);
			ms.Write(d0);
			ms.WriteUTF8Nullterm(flags);
			ms.WriteUTF8Nullterm(name);
			ms.WriteUTF8Nullterm(desc);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_mons : FileMod {
		public string GetDescription() {
			return "Fix text issues in enemy descriptions.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe1f521da8d1a98a4ul, 0xe9945859287da9cful, 0x2b0cdd38u));
			if (file_en == null) {
				return null;
			}
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			//for (int i = 0; i < tbl_en.Entries.Count; ++i) {
			//	var e = tbl_en.Entries[i];
			//	if (e.Name == "status") {
			//		var m = new MonsterData(e.Data);
			//		e.Data = m.ToBinary();
			//	}
			//}

			// Aion Type-Gamma II incorrectly claims that it wiped out Calvard's army. JP leaves it vague so do that too.
			{
				var m = new MonsterData(tbl_en.Entries[308].Data);
				m.desc = m.desc.Remove(51, 5);
				m.desc = m.desc.Insert(51, "S");
				m.desc = m.desc.ReplaceSubstring(60, 1, "\n", 0, 1);
				m.desc = m.desc.Insert(90, "n entire");
				m.desc = m.desc.InsertSubstring(98, m.desc, 120, 5);
				m.desc = m.desc.Remove(112, 18);
				tbl_en.Entries[308].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			DuplicatableByteArrayStream result_en_b = result_en.CopyToByteArrayStream();
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_mons.tbl", result_en_b.Duplicate())
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe1f521da8d1a98a4ul, 0xe9945859287da9cful, 0x2b0cdd38u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_mons.tbl", file_en)
			};
		}
	}
}
