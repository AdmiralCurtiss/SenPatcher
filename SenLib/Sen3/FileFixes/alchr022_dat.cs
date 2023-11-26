using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class alchr022_dat : FileMod {
		public string GetDescription() {
			return "Fix Rufus' Sword name";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2c6088c4f1e8847eul, 0x49f5f5f48b410fe2ul, 0xaec3ef54u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Holy Sword Ishnard -> Holy Sword of Ishnard
			bin.Position = 0x384;
			byte[] data = bin.ReadBytes(0x18);
			List<byte> newdata = new List<byte>();
			newdata.AddRange(data.Take(0xb));
			newdata.Add(0x6f);
			newdata.Add(0x66);
			newdata.AddRange(data.Skip(0xa).Take(8));
			bin.Position = 0x384;
			bin.Write(newdata.ToArray());

			return new FileModResult[] { new FileModResult("data/scripts/battle/dat_en/alchr022.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2c6088c4f1e8847eul, 0x49f5f5f48b410fe2ul, 0xaec3ef54u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/battle/dat_en/alchr022.dat", file) };
		}
	}
}
