using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t1010_dat : FileMod {
		public string GetDescription() {
			return "Voice fixes in Class 7 dorm.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xacbda7e71e5622feul, 0x0c600792f18d25adul, 0x2c4eca0au));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// remove sigh voice clip; this is weird to have because Rean is unvoiced in this scene otherwise
			patcher.RemovePartialCommand(0x2a14, 0x15, 0x2a17, 5);

			// a bunch of lines where they used duplicate takes across scenes instead of unique ones
			// there's two instances of the final bond scenes and the epilogue evening scenes here:
			// once for the memories menu and once for the actual game; a few have minor inconsistencies
			// that this fixes up too; also note that the epilogue evening scenes have two variants, for
			// if the person is picked as your partner or not picked as your partner, and the memories
			// menu only has duplicates for the former variant

			// laura (final bonding event), pc28v01138 -> pc28v01173
			bin.Position = 0x93e6;
			bin.Write(new byte[] { 0xfb, 0xfe });
			bin.Position = 0x39ace;
			bin.Write(new byte[] { 0xfb, 0xfe });

			// towa (final bonding event), pc28v01138 -> pc28v01325
			bin.Position = 0x15633;
			bin.Write(new byte[] { 0x44, 0xff });
			bin.Position = 0x4c6ab;
			bin.Write(new byte[] { 0x44, 0xff });

			// towa (final bonding event), pc28v01332 -> pc28v01341
			// inconsistent between game and memories menu, match memories to game
			bin.Position = 0x4dbdb;
			bin.Write(new byte[] { 0xf8, 0xfe });

			// towa (final bonding event), pc28v01333 -> pc28v01344
			bin.Position = 0x16e36;
			bin.Write(new byte[] { 0xf9, 0xfe });
			bin.Position = 0x4deae;
			bin.Write(new byte[] { 0xf9, 0xfe });

			// machias (final bonding event), single glyph ... -> three dot ...
			bin.Position = 0x1d581;
			bin.Write(new byte[] { 0x2e, 0x2e, 0x2e });
			bin.Position = 0x55585;
			bin.Write(new byte[] { 0x2e, 0x2e, 0x2e });

			// gaius (final bonding event), pc28v01138 -> pc28v01447
			bin.Position = 0x20ece;
			bin.Write(new byte[] { 0xfa, 0xfe });
			bin.Position = 0x5b14a;
			bin.Write(new byte[] { 0xfa, 0xfe });

			// sharon (epilogue evening), pc28v01807 -> pc28v01644
			bin.Position = 0x2371f;
			bin.Write(new byte[] { 0x45, 0xff });

			// jusis (epilogue evening), pc28v01676 -> pc28v01694
			bin.Position = 0x27066;
			bin.Write(new byte[] { 0x46, 0xff });
			bin.Position = 0x5a4a4;
			bin.Write(new byte[] { 0x46, 0xff });

			// laura (epilogue evening), pc28v01707 -> pc28v01774
			bin.Position = 0x2ca9c;
			bin.Write(new byte[] { 0xff, 0xfe });
			bin.Position = 0x3cb18;
			bin.Write(new byte[] { 0xff, 0xfe });

			// laura (epilogue evening), pc28v01648 -> pc28v01779
			bin.Position = 0x2cefd;
			bin.Write(new byte[] { 0x01, 0xff });
			bin.Position = 0x3cf79;
			bin.Write(new byte[] { 0x01, 0xff });

			// emma (epilogue evening), pc28v01707 -> pc28v01796
			bin.Position = 0x2e27e;
			bin.Write(new byte[] { 0x47, 0xff });
			bin.Position = 0x4034b;
			bin.Write(new byte[] { 0x47, 0xff });

			// emma (epilogue evening), pc28v01648 -> pc28v01800
			bin.Position = 0x2e85e;
			bin.Write(new byte[] { 0x48, 0xff });
			bin.Position = 0x4092b;
			bin.Write(new byte[] { 0x48, 0xff });

			// fie (epilogue evening if NOT picked), pc28v01770 -> pc28v01812
			bin.Position = 0x2f56e;
			bin.Write(new byte[] { 0x05, 0xff });

			// fie (epilogue evening), pc28v01648 -> pc28v01817
			// inconsistent between game and memories menu, match memories to game
			bin.Position = 0x43b64;
			bin.Write(new byte[] { 0x06, 0xff });

			// fie (epilogue evening), pc28v01707 -> pc28v01819
			bin.Position = 0x2fcbe;
			bin.Write(new byte[] { 0x07, 0xff });
			bin.Position = 0x43ca5;
			bin.Write(new byte[] { 0x07, 0xff });

			// fie (epilogue evening), pc28v01676 -> pc28v01822
			bin.Position = 0x2fe07;
			bin.Write(new byte[] { 0x09, 0xff });
			bin.Position = 0x43dee;
			bin.Write(new byte[] { 0x09, 0xff });

			// millium (epilogue evening), pc28v01648 -> pc28v01835
			// inconsistent between game and memories menu, match memories to game
			bin.Position = 0x47025;
			bin.Write(new byte[] { 0x2a, 0xff });

			// sara (epilogue evening), pc28v01648 -> pc28v01861
			bin.Position = 0x325e6;
			bin.Write(new byte[] { 0x0a, 0xff });
			bin.Position = 0x4b173;
			bin.Write(new byte[] { 0x0a, 0xff });


			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1010.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xacbda7e71e5622feul, 0x0c600792f18d25adul, 0x2c4eca0au));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1010.dat", file) };
		}
	}
}
