using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3510_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Ordis (Duke's Castle Entrance).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7d6eddb306de371ful, 0xd7fcfd55b5993016ul, 0x83e18853u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// the port city -> the Port City
			bin.Position = 0x4401;
			bin.WriteUInt8(0x50);
			bin.Position = 0x4406;
			bin.WriteUInt8(0x43);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3510.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7d6eddb306de371ful, 0xd7fcfd55b5993016ul, 0x83e18853u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3510.dat", file) };
		}
	}
}
