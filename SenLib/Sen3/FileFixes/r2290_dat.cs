using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r2290_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (Ex. Camp).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0c5fbbb90cb3459eul, 0x0d2f5b9769eb5f7dul, 0x9d87992cu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// royal family -> Imperial family
			patcher.ReplacePartialCommand(0x188ae, 0x53, 0x188d2, 0x3, new byte[] {0x49, 0x6d, 0x70, 0x65, 0x72, 0x69});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r2290.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0c5fbbb90cb3459eul, 0x0d2f5b9769eb5f7dul, 0x9d87992cu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r2290.dat", file) };
		}
	}
}
