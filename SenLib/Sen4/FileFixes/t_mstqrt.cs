using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
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
			return "Fix one incorrect Master Quartz effect description.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x40ae4f525021b550ul, 0xb77e045a00841a42ul, 0xbc460a77u));
			if (file_en == null) {
				return null;
			}

			var tbl_en = new Tbl(file_en, EndianUtils.Endianness.LittleEndian);

			// Katze says attack/craft instead of magic for its second effect
			{
				int idx = 323;
				var m = new MasterQuartzMemo(tbl_en.Entries[idx].Data);
				m.str = m.str.ReplaceSubstring(1, 14, new MasterQuartzMemo(tbl_en.Entries[325].Data).str, 1, 14);
				tbl_en.Entries[idx].Data = m.ToBinary();
			}

			Stream result_en = new MemoryStream();
			tbl_en.WriteToStream(result_en, EndianUtils.Endianness.LittleEndian);
			return new FileModResult[] { new FileModResult("data/text/dat_en/t_mstqrt.tbl", result_en) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file_en = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x40ae4f525021b550ul, 0xb77e045a00841a42ul, 0xbc460a77u));
			if (file_en == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/text/dat_en/t_mstqrt.tbl", file_en) };
		}
	}
}
