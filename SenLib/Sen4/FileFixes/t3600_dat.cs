using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4.FileFixes {
	class t3600_dat : FileMod {
		public string GetDescription() {
			return "Typo in Casper's name.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x21ac0c6c99dcf57cul, 0x0f75b00d44d42d8dul, 0xae99fe3fu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Caspar -> Casper
			bin.Position = 0x41e8;
			bin.WriteByte(0x65);
			bin.Position = 0x4289;
			bin.WriteByte(0x65);
			bin.Position = 0x4502;
			bin.WriteByte(0x65);
			bin.Position = 0x45a3;
			bin.WriteByte(0x65);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3600.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x21ac0c6c99dcf57cul, 0x0f75b00d44d42d8dul, 0xae99fe3fu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t3600.dat", file) };
		}
	}
}
