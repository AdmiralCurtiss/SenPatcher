using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c0250_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Crossbell (SSS Building).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe9ef91d4cf274a0ful, 0xb055303c3d29dcd1ul, 0xaf99fd49u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// old battlefield -> ancient battlefield
			patcher.ReplacePartialCommand(0x34a3, 0xc4, 0x3502, 0x3, new byte[] {0x61, 0x6e, 0x63, 0x69, 0x65, 0x6e, 0x74});
			patcher.ReplacePartialCommand(0x58d2, 0x136, 0x596d, 0x3, new byte[] {0x61, 0x6e, 0x63, 0x69, 0x65, 0x6e, 0x74});

			// royal family -> Imperial family
			patcher.ReplacePartialCommand(0x6ef4, 0x26, 0x6f07, 0x3, new byte[] {0x49, 0x6d, 0x70, 0x65, 0x72, 0x69});
			patcher.ReplacePartialCommand(0x6f34, 0x5f, 0x6f70, 0x3, new byte[] {0x49, 0x6d, 0x70, 0x65, 0x72, 0x69});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0250.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe9ef91d4cf274a0ful, 0xb055303c3d29dcd1ul, 0xaf99fd49u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c0250.dat", file) };
		}
	}
}
