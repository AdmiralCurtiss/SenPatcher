using HyoutaUtils;
using System.Collections.Generic;
using System.IO;


namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t3500_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in main Roer area.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd18f9880c045b969ul, 0xafd8c6a8836ee6e8ul, 0x6810aa4eu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			bin.Position = 0x9ce6;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x28b20;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x29b60;
			bin.WriteUInt24(0x2e2e2e);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3500.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd18f9880c045b969ul, 0xafd8c6a8836ee6e8ul, 0x6810aa4eu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t3500.dat", s) };
		}
	}
}
