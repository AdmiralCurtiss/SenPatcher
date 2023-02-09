using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_system_dat : FileMod {
		public string GetDescription() {
			return "Fix grammar error in message when reporting cryptids.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2a5b55ee43ae4498ul, 0xeccb409a491c7a6eul, 0xa1e4471au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// 'been' -> 'is'
			patcher.ReplacePartialCommand(0xfcc4, 0x82, 0xfd01, 0x4, new byte[] {0x69, 0x73});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/system.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2a5b55ee43ae4498ul, 0xeccb409a491c7a6eul, 0xa1e4471au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/system.dat", file) };
		}
	}
}
