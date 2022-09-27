using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0830_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (Heiyue).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xbb6a5b0a31ebe05cul, 0xeb06ae8c0b773e2ful, 0x0b8823afu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// Heiyue Trading Company -> Heiyue trading company
			bin.Position = 0x19bc;
			bin.WriteUInt8(0x74);
			bin.Position = 0x19c4;
			bin.WriteUInt8(0x63);
			bin.Position = 0x685e;
			bin.WriteUInt8(0x74);
			bin.Position = 0x6866;
			bin.WriteUInt8(0x63);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0830.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xbb6a5b0a31ebe05cul, 0xeb06ae8c0b773e2ful, 0x0b8823afu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0830.dat", file) };
		}
	}
}
