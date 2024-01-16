using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0000_dat : FileMod {
		public string GetDescription() {
			return "Fix formatting issues on Thors campus grounds.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x1c6e0, 0x8b, 0x1c71f, 3, new byte[] { 0x27 });
			patcher.ReplacePartialCommand(0x1c6e0, 0x89, 0x1c74f, 3, new byte[] { 0x27 });

			// formatting issues in Alisa Chapter 1 Evening bonding event
			bin.SwapBytes(0x79307, 0x7930d);

			// formatting issues in Jusis Chapter 3 Day bonding event
			bin.SwapBytes(0x7c3a8, 0x7c3ab);
			bin.Position = 0x7c3cd;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7c4b5, 0x7c4bb);
			bin.SwapBytes(0x7c55a, 0x7c55f);
			bin.SwapBytes(0x7c585, 0x7c58a);
			bin.SwapBytes(0x7c651, 0x7c659);
			bin.SwapBytes(0x7c908, 0x7c910);
			bin.SwapBytes(0x7c957, 0x7c95d);
			bin.SwapBytes(0x7c9be, 0x7c9c2);
			bin.SwapBytes(0x7ce6f, 0x7ce74);

			// formatting issues in Alisa Chapter 5 Day bonding event
			bin.SwapBytes(0x79ad0, 0x79ad6);
			bin.SwapBytes(0x7a484, 0x7a488);
			bin.SwapBytes(0x7a4ab, 0x7a4b3);
			bin.SwapBytes(0x7a598, 0x7a59d);

			// formatting issues in Fie Chapter 5 Evening bonding event
			bin.SwapBytes(0x7f942, 0x7f947);
			bin.Position = 0x7fcc6;
			bin.WriteUInt8(0x01);

			// formatting issues in Fie Chapter 6 Day bonding event
			bin.SwapBytes(0x8024c, 0x80251);
			bin.SwapBytes(0x8080e, 0x80814);

			// formatting issues in Elliot Chapter 6 Day bonding event
			bin.Position = 0x7dec6;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7df0f, 0x7df13);
			bin.SwapBytes(0x7df36, 0x7df3b);
			bin.SwapBytes(0x7df7d, 0x7df87);
			bin.SwapBytes(0x7dff0, 0x7dff6);
			bin.SwapBytes(0x7e05d, 0x7e066);
			bin.Position = 0x7e07f;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7e14e, 0x7e151);

			// formatting issues in Alisa Chapter 6 Evening bonding event
			bin.Position = 0x7adaf;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7af31, 0x7af36);
			bin.SwapBytes(0x7b2a5, 0x7b2ae);
			bin.Position = 0x7b2ca;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7b3e2, 0x7b3e6);
			bin.Position = 0x7b407;
			bin.WriteUInt8(0x01);
			bin.SwapBytes(0x7b8f2, 0x7b8f5);
			bin.SwapBytes(0x7b91b, 0x7b91f);
			bin.SwapBytes(0x7b98d, 0x7b991);
			bin.SwapBytes(0x7b9e5, 0x7b9ea);
			bin.SwapBytes(0x7ba07, 0x7ba0f);

			// fix several lines in the side quests at the festival, this really got messed up somehow
			patcher.ReplacePartialCommand(0xa1c1a, 0x40, 0xa1c20, 0x17, new byte[] { 0x4f, 0x68, 0x2c, 0x20, 0x49, 0x20, 0x73, 0x65, 0x65, 0x2e, 0x2e });
			patcher.ReplacePartialCommand(0xa2cf9, 0x1f, 0xa2cff, 0x16, new byte[] { 0x45, 0x78, 0x63, 0x75, 0x73, 0x65, 0x20, 0x6d, 0x65, 0x2c, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x73, 0x6f, 0x6e, 0x27, 0x73, 0x01, 0x62, 0x61, 0x6c, 0x6c, 0x6f, 0x6f, 0x6e, 0x20, 0x75, 0x70, 0x20, 0x74, 0x68, 0x65, 0x72, 0x65 });
			patcher.ReplacePartialCommand(0xa353a, 0x41, 0xa3578, 0x1, new byte[] { 0x01, 0x61, 0x6e, 0x79, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x20, 0x65, 0x6c, 0x73, 0x65, 0x2e });

			// fix lowercase 't' at start of sentence
			bin.Position = 0xa3ebc;
			bin.WriteByte(0x54);

			// course rock salt -> coarse rock salt
			bin.Position = 0xa25d2;
			bin.WriteByte(0x61);

			// 'or really good guy' -> 'or a really good guy' plus textbox formatting
			using (MemoryStream ms = new MemoryStream()) {
				bin.Position = 0x21b9b;
				byte[] data = bin.ReadBytes(0x78);
				ms.Write(data, 0, 0x4f);
				ms.Write(data, 0x50, 0x4);
				ms.WriteByte(0x01);
				ms.Write(data, 0x55, 0xd);
				ms.WriteByte(0x61);
				ms.Write(data, 0x61, 0x17);
				patcher.ReplaceCommand(0x21b9b, 0x78, ms.CopyToByteArrayAndDispose());
			}

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x83fc174bcce22201ul, 0xfe2053f855e8879bul, 0x3091e649u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t0000.dat", s) };
		}
	}
}
