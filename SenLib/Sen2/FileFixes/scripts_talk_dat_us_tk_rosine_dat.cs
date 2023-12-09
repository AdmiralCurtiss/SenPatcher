using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_talk_dat_us_tk_rosine_dat : FileMod {
		public string GetDescription() {
			return "Fix typos in conversations with Rosine.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xea737c88d0648621ul, 0xc297ed0c139348aaul, 0x6213a40du));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// extra comma
			patcher.RemovePartialCommand(0x2506, 0x4e, 0x253f, 0x1);

			// fully fledged -> fully-fledged
			bin.Position = 0x2627;
			bin.Write(new byte[] {0x2d});

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_rosine.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xea737c88d0648621ul, 0xc297ed0c139348aaul, 0x6213a40du));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_rosine.dat", file) };
		}
	}
}
