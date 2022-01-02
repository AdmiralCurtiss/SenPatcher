using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1000_dat : FileMod {
		public string GetDescription() {
			return "Fix continuity error in chapter 3 and a few formatting issues in the main Trista area.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x84d3de50b7318f20ul, 0xb4fe48836404d134ul, 0xa124be52u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// fix line from Alisa about the upcoming exams
			// this has been fixed in the PS4 version but for some reason was not patched back into the PC version
			patcher.ReplacePartialCommand(0x25b3e, 0x38, 0x25b59, 0x14, new byte[] {
				0x66, 0x65, 0x65, 0x6c, 0x3f, 0x20, 0x41, 0x6c, 0x6c, 0x01, 0x70, 0x72, 0x65, 0x70, 0x70, 0x65,
				0x64, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x72, 0x65, 0x61, 0x64, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
				0x6f, 0x75, 0x72
			});

			// two lines later, linebreak was moved to a nicer spot, might as well apply that too
			bin.SwapBytes(0x25bee, 0x25bfb);

			// formatting issues in Machias Chapter 1 Day bonding event
			bin.SwapBytes(0x39da5, 0x39dac);
			bin.Position = 0x39f87;
			bin.WriteUInt8(0x01);

			// formatting issues in Fie Chapter 3 Day bonding event
			bin.Position = 0x3914a;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x391b3, 0x391b8);
			bin.SwapBytes(0x39202, 0x39208);
			bin.Position = 0x39227;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x39430, 0x39436);
			bin.SwapBytes(0x3945b, 0x3945f);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x84d3de50b7318f20ul, 0xb4fe48836404d134ul, 0xa124be52u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1000.dat", s) };
		}
	}
}
