using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0020_dat : FileMod {
		public string GetDescription() {
			return "Fix minor formatting issues in Thors (top floor).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9668b944717fe228ul, 0x3a482367f1448ee1ul, 0xfc63e832u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			// formatting issues in Gaius Chapter 2 Day bonding event
			bin.SwapBytes(0x58760, 0x58764);
			bin.SwapBytes(0x58915, 0x58919);
			bin.SwapBytes(0x589c3, 0x589c8);
			bin.SwapBytes(0x589f9, 0x589fd);
			bin.SwapBytes(0x58a20, 0x58a26);

			// formatting issues in Fie Chapter 2 Evening bonding event
			bin.SwapBytes(0x577b4, 0x577b7);
			bin.Position = 0x57c0c;
			bin.WriteUInt8(0x01);

			// formatting issues in Elliot Chapter 3 Day bonding event
			bin.SwapBytes(0x55e76, 0x55e7d);
			bin.SwapBytes(0x56007, 0x5600c);
			bin.Position = 0x561fb;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x56238, 0x5623d);
			bin.SwapBytes(0x5627b, 0x56280);
			bin.SwapBytes(0x563be, 0x563c2);

			// formatting issues in Alisa Chapter 3 Day bonding event
			bin.SwapBytes(0x550aa, 0x550b1);
			bin.SwapBytes(0x550d0, 0x550dc);

			// formatting issues in Elliot Chapter 4 Evening bonding event
			bin.SwapBytes(0x56913, 0x56919);
			bin.SwapBytes(0x5693c, 0x56940);
			bin.SwapBytes(0x56af4, 0x56af7);
			bin.SwapBytes(0x56b35, 0x56b3a);
			bin.SwapBytes(0x56d2a, 0x56d37);
			bin.SwapBytes(0x56d54, 0x56d63);
			bin.SwapBytes(0x5729d, 0x572a1);

			// formatting issues in Gaius Chapter 6 Day bonding event
			bin.SwapBytes(0x59357, 0x5935c);
			bin.SwapBytes(0x59482, 0x59486);
			bin.SwapBytes(0x5972f, 0x59736);
			bin.SwapBytes(0x5976f, 0x59778);

			// formatting issues in Crow Chapter 6 Evening bonding event
			bin.SwapBytes(0x59f0a, 0x59f11);
			bin.SwapBytes(0x5a022, 0x5a027);
			bin.Position = 0x5a048;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x5a1cb, 0x5a1ce);
			bin.SwapBytes(0x5a1f3, 0x5a1fa);
			bin.SwapBytes(0x5a240, 0x5a245);
			bin.SwapBytes(0x5a47b, 0x5a480);
			bin.SwapBytes(0x5aaa0, 0x5aaa6);
			bin.SwapBytes(0x5aac6, 0x5aad1);
			bin.SwapBytes(0x5ac44, 0x5ac48);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0020.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9668b944717fe228ul, 0x3a482367f1448ee1ul, 0xfc63e832u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0020.dat", s) };
		}
	}
}
