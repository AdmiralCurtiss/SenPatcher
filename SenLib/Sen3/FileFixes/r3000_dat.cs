using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r3000_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in West Lamare Highway 2.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9bde3d287daabc7dul, 0xf76207d20b04da2aul, 0xd1c7120au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// port city -> Port City
			bin.Position = 0x40c3;
			bin.WriteUInt8(0x50);
			bin.Position = 0x40c8;
			bin.WriteUInt8(0x43);

			// The Juno Naval Fortress -> Juno Naval Fortress
			patcher.RemovePartialCommand(0xbd09, 0x94, 0xbd19, 0x4);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9bde3d287daabc7dul, 0xf76207d20b04da2aul, 0xd1c7120au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3000.dat", file) };
		}
	}
}
