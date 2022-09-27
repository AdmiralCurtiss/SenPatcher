using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class m0600_dat : FileMod {
		public string GetDescription() {
			return "Capitalization fixes in Isthmia.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x09161c59396b4aa8ul, 0x932f628fa9709ca8ul, 0x899fe58cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Time/Space/Mirage -> time/space/mirage
			bin.Position = 0x2de5;
			bin.WriteUInt8(0x74);
			bin.Position = 0x2deb;
			bin.WriteUInt8(0x73);
			bin.Position = 0x2df6;
			bin.WriteUInt8(0x6d);
			bin.Position = 0x2fb3;
			bin.WriteUInt8(0x74);
			bin.Position = 0x2fb9;
			bin.WriteUInt8(0x73);
			bin.Position = 0x2fc4;
			bin.WriteUInt8(0x6d);
			bin.Position = 0xb1e6;
			bin.WriteUInt8(0x74);
			bin.Position = 0xb1ec;
			bin.WriteUInt8(0x73);
			bin.Position = 0xb1f7;
			bin.WriteUInt8(0x6d);
			bin.Position = 0xb265;
			bin.WriteUInt8(0x74);
			bin.Position = 0xb26b;
			bin.WriteUInt8(0x73);
			bin.Position = 0xb276;
			bin.WriteUInt8(0x6d);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0600.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x09161c59396b4aa8ul, 0x932f628fa9709ca8ul, 0x899fe58cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/m0600.dat", file) };
		}
	}
}
