using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t3400_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Ordis (Noble District).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd48fd0f978975b3dul, 0x4854d72475ba6700ul, 0x76961db8u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// the port city -> the Port City
			bin.Position = 0xcb69;
			bin.WriteUInt8(0x50);
			bin.Position = 0xcb6e;
			bin.WriteUInt8(0x43);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3400.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd48fd0f978975b3dul, 0x4854d72475ba6700ul, 0x76961db8u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3400.dat", file) };
		}
	}
}
