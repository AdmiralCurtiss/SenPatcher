using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_talk_dat_us_tk_monica_dat : FileMod {
		public string GetDescription() {
			return "Fix reference to incorrect character in dialogue with Monica.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x836140611baf2149ul, 0xc7ac1a28182762b2ul, 0x5354b54fu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Hibelle -> Casper
			// Whoever edited or translated this must have been really tired here...
			bin.Position = 0x17c4;
			byte[] casper = bin.ReadBytes(6);
			patcher.ReplacePartialCommand(0x4b5, 0x83, 0x4dc, 0x7, casper);

			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_monica.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x836140611baf2149ul, 0xc7ac1a28182762b2ul, 0x5354b54fu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/talk/dat_us/tk_monica.dat", file) };
		}
	}
}
