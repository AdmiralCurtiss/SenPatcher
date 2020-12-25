using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0010_dat : FileMod {
		public string GetDescription() {
			return "Fix double space in Chapter 6 scene in front of school building.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0xfbbf, 0x53, 0xfbcf, 1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8a76ff88baf96b5eul, 0x72e675d0d5d3b75aul, 0x72cc3989u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0010.dat", s) };
		}
	}
}
