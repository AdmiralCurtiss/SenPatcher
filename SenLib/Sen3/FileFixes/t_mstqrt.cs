using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class MasterQuartzMemo {
		public ushort mqidx;
		public ushort stridx;
		public string str;

		internal MasterQuartzMemo(byte[] data) {
			var stream = new DuplicatableByteArrayStream(data);
			mqidx = stream.ReadUInt16();
			stridx = stream.ReadUInt16();
			str = stream.ReadUTF8Nullterm();
		}

		internal byte[] ToBinary() {
			MemoryStream ms = new MemoryStream();
			ms.WriteUInt16(mqidx);
			ms.WriteUInt16(stridx);
			ms.WriteUTF8Nullterm(str);
			return ms.CopyToByteArrayAndDispose();
		}
	}

	class t_mstqrt : FileMod {
		public string GetDescription() {
			return "Fix a handful of Master Quartzes having broken effects when playing in English or French.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_jp = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x86c4d62ac6efc4abul, 0x5dbfdc7f5bf36c4eul, 0xaef64c3bu));
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x494b68fcbc463581ul, 0xf79f7dd1f6444df9ul, 0xc4ad6204u));
			var file_fr = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb8022803ece579b5ul, 0x342e5143dc91ae47ul, 0x06fd7f7cu));
			if (file_jp == null || file_en == null || file_fr == null) {
				return null;
			}

			var tbl_jp = new Tbl(file_jp, EndianUtils.Endianness.LittleEndian);
			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);
			var tbl_fr = new Tbl(file_fr, EndianUtils.Endianness.LittleEndian);

			// copy over the correct effect data from the JP file
			for (int i = 0; i < tbl_jp.Entries.Count; ++i) {
				if (tbl_jp.Entries[i].Name == "MasterQuartzData") {
					tbl_en.Entries[i].Data = tbl_jp.Entries[i].Data;
					tbl_fr.Entries[i].Data = tbl_jp.Entries[i].Data;
				}
			}

			// attach sentinel '$' to the strings where the values need to be swapped
			AttachSentinel(tbl_fr, 67);
			foreach (Tbl tbl in new Tbl[] { tbl_en, tbl_fr }) {
				AttachSentinel(tbl, 123);
				AttachSentinel(tbl, 137);
				AttachSentinel(tbl, 193);
				AttachSentinel(tbl, 221);
				AttachSentinel(tbl, 291);
				AttachSentinel(tbl, 347);
				AttachSentinel(tbl, 391);
			}

			// also while we're here fix this missing newline in the FR file...
			tbl_fr.Entries[66].Data[tbl_fr.Entries[66].Data.Length - 2] = 0x0a;

			// Virgo lists EP healing instead of HP
			{
				var m = new MasterQuartzMemo(tbl_en.Entries[263].Data);
				m.str = m.str.Replace("EP", "HP");
				tbl_en.Entries[263].Data = m.ToBinary();
			}

			// For clarity mention that Breaking is okay too for the 2x drop
			{
				var m = new MasterQuartzMemo(tbl_en.Entries[393].Data);
				m.str = m.str.InsertSubstring(54, "breaks or ", 0, 10);
				tbl_en.Entries[393].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			Stream result_fr = new MemoryStream();
			tbl_fr.WriteToStream(result_fr, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_mstqrt.tbl", result_en),
				new FileModResult("data/text/dat_fr/t_mstqrt.tbl", result_fr)
			};
		}

		private static void AttachSentinel(Tbl tbl, int index) {
			var l = tbl.Entries[index].Data.ToList();
			l.Insert(4, 0x24);
			tbl.Entries[index].Data = l.ToArray();
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x494b68fcbc463581ul, 0xf79f7dd1f6444df9ul, 0xc4ad6204u));
			var file_fr = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xb8022803ece579b5ul, 0x342e5143dc91ae47ul, 0x06fd7f7cu));
			if (file_en == null || file_fr == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/text/dat_en/t_mstqrt.tbl", file_en),
				new FileModResult("data/text/dat_fr/t_mstqrt.tbl", file_fr)
			};
		}
	}
}
