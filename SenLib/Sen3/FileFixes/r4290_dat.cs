using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r4290_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Heimdallr Ex. Camp.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x871c9c02460dde4aul, 0xcbb7712111af384eul, 0xd76a3bdcu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// General Aurelia -> General Le Guin
			bin.Position = 0x15ef0;
			bin.Write(new byte[] {0x4c, 0x65, 0x20, 0x47, 0x75, 0x69, 0x6e});

			// General Aurelia -> Aurelia
			patcher.RemovePartialCommand(0x13d0d, 0x53, 0x13d19, 8);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r4290.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x871c9c02460dde4aul, 0xcbb7712111af384eul, 0xd76a3bdcu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r4290.dat", file) };
		}
	}
}
