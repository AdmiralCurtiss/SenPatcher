using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_scena_dat_us_t4080_dat : FileMod {
		public string GetDescription() {
			return "Fix various minor English voice issues (missing and wrong voice clips) in hot spring scenes.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x18c7ad19818f25d7ul, 0xa2ad61c3f7c711bful, 0x16cff933u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();

			var patcher = new SenScriptPatcher(bin);

			// Elise in Prologue
			patcher.ExtendPartialCommand(0x3176, 0x8, 0x3179, new byte[] { 0x11, 0x2b, 0xff, 0x00, 0x00 });
			patcher.ExtendPartialCommand(0x34bc, 0x31, 0x34d8, new byte[] { 0x11, 0x2c, 0xff, 0x00, 0x00 });

			// Emma
			bin.Position = 0xf678;
			bin.WriteUInt16(65326, EndianUtils.Endianness.LittleEndian);
			patcher.ExtendPartialCommand(0xf674, 0x11c, 0xf747, new byte[] { 0x11, 0x2f, 0xff, 0x00, 0x00 });
			bin.Position = 0xf9a0;
			bin.WriteUInt16(65328, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0xf9d2;
			bin.WriteUInt16(65329, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0xfa0c;
			bin.WriteUInt16(65330, EndianUtils.Endianness.LittleEndian);

			// Jusis
			bin.Position = 0x113a2;
			bin.WriteUInt16(65331, EndianUtils.Endianness.LittleEndian);

			// Fie
			bin.Position = 0x12bb8;
			bin.WriteUInt16(65332, EndianUtils.Endianness.LittleEndian);

			// Gaius
			bin.Position = 0x14208;
			bin.WriteUInt16(65333, EndianUtils.Endianness.LittleEndian);

			// Claire
			bin.Position = 0x18718;
			bin.WriteUInt16(65334, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x18dc4;
			bin.WriteUInt16(65335, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x18df8;
			bin.WriteUInt16(65336, EndianUtils.Endianness.LittleEndian);

			// Sharon
			bin.Position = 0x1a18f;
			bin.WriteUInt16(65337, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x1a820;
			bin.WriteUInt16(65338, EndianUtils.Endianness.LittleEndian);

			// Sara
			bin.Position = 0x1bc08;
			bin.WriteUInt16(65339, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x1bc72;
			bin.WriteUInt16(65340, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x1c1d1;
			bin.WriteUInt16(65341, EndianUtils.Endianness.LittleEndian);
			bin.Position = 0x1c205;
			bin.WriteUInt16(65342, EndianUtils.Endianness.LittleEndian);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4080.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x18c7ad19818f25d7ul, 0xa2ad61c3f7c711bful, 0x16cff933u));
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4080.dat", s) };
		}
	}
}
