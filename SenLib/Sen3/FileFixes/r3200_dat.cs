using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r3200_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Auros Coastal Road.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x48c59f32ff001518ul, 0xdcef78c47b5f2050ul, 0xf3e0de55u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// port city -> Port City
			bin.Position = 0x2db1;
			bin.WriteUInt8(0x50);
			bin.Position = 0x2db6;
			bin.WriteUInt8(0x43);
			bin.Position = 0x4bac;
			bin.WriteUInt8(0x50);
			bin.Position = 0x4bb1;
			bin.WriteUInt8(0x43);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x48c59f32ff001518ul, 0xdcef78c47b5f2050ul, 0xf3e0de55u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3200.dat", file) };
		}
	}
}
