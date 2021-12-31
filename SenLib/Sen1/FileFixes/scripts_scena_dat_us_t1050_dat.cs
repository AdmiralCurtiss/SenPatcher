using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1050_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Kirsche's.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x60ff5f6535daa1deul, 0xbc7ecab2d514a387ul, 0xeeecb587u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Crow Chapter 2 Day bonding event
			bin.SwapBytes(0x12665, 0x12669);
			bin.Position = 0x1268e;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x128c6, 0x128cd);
			bin.Position = 0x128e8;
			bin.WriteUInt8(0x01);
			bin.Position = 0x12a3e;
			bin.WriteUInt8(0x01);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1050.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x60ff5f6535daa1deul, 0xbc7ecab2d514a387ul, 0xeeecb587u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1050.dat", s) };
		}
	}
}
