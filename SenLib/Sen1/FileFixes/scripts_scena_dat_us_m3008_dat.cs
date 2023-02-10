using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	class scripts_scena_dat_us_m3008_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in final dungeon.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf44b397573f11277ul, 0x11f66ec631b2de5dul, 0xffdafc38u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();

			// I'm Glad -> I'm glad
			bin.Position = 0x97c4;
			bin.WriteByte(0x67);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m3008.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf44b397573f11277ul, 0x11f66ec631b2de5dul, 0xffdafc38u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/m3008.dat", file) };
		}
	}
}
