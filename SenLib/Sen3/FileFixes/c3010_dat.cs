using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class c3010_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Heimdallr (Cathedral).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x648885e17c3f2371ul, 0xf9a71b07990e054dul, 0xb09242fbu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Sept-Terrion -> Sept-Terrions
			patcher.ExtendPartialCommand(0xed1b, 0x4e, 0xed66, new byte[] {0x73});
			patcher.ExtendPartialCommand(0xf24e, 0x10a, 0xf2e9, new byte[] {0x73});

			// an immortal -> an Immortal
			bin.Position = 0x124da;
			bin.Write(new byte[] {0x49});

			// an immortal -> immortal
			patcher.RemovePartialCommand(0x12feb, 0xfd, 0x13015, 0x3);

			// removed 'but...'
			patcher.RemovePartialCommand(0x13147, 0x3d, 0x1317a, 0x7);

			// Sir Lysander! Sir Gaius! -> Father Thomas! Father Gaius!
			bin.SwapBytes(0x1d992, 0x1d99a);
			patcher.ReplacePartialCommand(0x1d958, 0x65, 0x1d967, 0x10, new byte[] { 0x46, 0x61, 0x74, 0x68, 0x65, 0x72, 0x20, 0x54, 0x68, 0x6f, 0x6d, 0x61, 0x73, 0x21, 0x20, 0x46, 0x61, 0x74, 0x68, 0x65 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x648885e17c3f2371ul, 0xf9a71b07990e054dul, 0xb09242fbu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/c3010.dat", file) };
		}
	}
}
