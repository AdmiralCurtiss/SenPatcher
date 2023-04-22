using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0420_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (Fisherman's Guild).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4d4183d8d45f846cul, 0x7659a7db1d8c39c7ul, 0x58ffb50au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// expert angler -> Master Fisher
			bin.Position = 0x1837;
			bin.Write(new byte[] {0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x46, 0x69, 0x73, 0x68});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0420.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4d4183d8d45f846cul, 0x7659a7db1d8c39c7ul, 0x58ffb50au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0420.dat", file) };
		}
	}
}
