using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0032_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in teahouse festival events.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe7854ac057166d50ul, 0xd94c340ec39403d2ul, 0x6173ff9fu));
			if (s == null) {
				return null;
			}

			MemoryStream bin = s.CopyToMemoryAndDispose();
			var patcher = new SenScriptPatcher(bin);

			bin.Position = 0x2360;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0xe22a;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x11fde;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x137db;
			bin.WriteUInt24(0x2e2e2e);

			// 'no way I can to quit now' -> 'no way I can quit now'
			patcher.RemovePartialCommand(0x1622b, 0xa5, 0x162c2, 0x3);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0032.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe7854ac057166d50ul, 0xd94c340ec39403d2ul, 0x6173ff9fu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0032.dat", s) };
		}
	}
}
