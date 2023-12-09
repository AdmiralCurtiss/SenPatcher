using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t3060_dat : FileMod {
		public string GetDescription() {
			return "Text fixes in Viscount Arseid's Mansion.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7da332f0c421b679ul, 0x6b4c2e0cc53b570cul, 0x4a4d974eu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// remove duplicate 'how'
			patcher.RemovePartialCommand(0x3f00, 0x119, 0x3faa, 0x4);

			// add voice clip
			patcher.ExtendPartialCommand(0x67c0, 0xd, 0x67c8, new byte[] { 0x11, 0x4e, 0xff, 0x00, 0x00 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3060.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x7da332f0c421b679ul, 0x6b4c2e0cc53b570cul, 0x4a4d974eu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3060.dat", file) };
		}
	}
}
