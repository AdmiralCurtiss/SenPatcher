using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1050_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues in Kirsche's.";
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

			// formatting issues in Machias Chapter 4 Day bonding event
			bin.SwapBytes(0xf9f8, 0xf9fa);
			bin.SwapBytes(0xfa9f, 0xfaa8);
			bin.SwapBytes(0xfac4, 0xfad2);
			bin.SwapBytes(0xfb8d, 0xfb93);
			bin.SwapBytes(0xfe48, 0xfe4d);
			bin.SwapBytes(0xfe98, 0xfe9e);
			bin.SwapBytes(0x100a9, 0x100b0);
			bin.SwapBytes(0x100cf, 0x100d8);

			// formatting issues in Gaius Chapter 5 Day bonding event
			bin.SwapBytes(0x109d0, 0x109d3);
			bin.Position = 0x10c48;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x10d5b, 0x10d60);
			bin.SwapBytes(0x117bf, 0x117c3);

			// formatting issues in Sara Chapter 6 Day bonding event
			bin.Position = 0x13713;
			bin.WriteUInt8(0x01);
			bin.Position = 0x137d7;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x138e7, 0x138ec);
			bin.Position = 0x139a5;
			bin.WriteUInt8(0x01);
			bin.Position = 0x13b38;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x13bce, 0x13bd2);

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
