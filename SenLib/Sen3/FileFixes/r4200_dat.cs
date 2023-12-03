using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r4200_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in South Ostia Highway.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd6dcc55f71cf2e61ul, 0x93c6a33b53b8879cul, 0x0d4d5958u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// Hmph, so the Divine Angler of Erebonia has been{n}chosen... I have another rival now.
			// -> Hmph, you may end up as Erebonia's Divine Angler...{n}It seems I have another rival now.
			using (MemoryStream ms = new MemoryStream()) {
				ms.Write(bin.ReadBytesFromLocationAndReset(0x27ed, 4));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x7e3c, 4));
				ms.Write(bin.ReadBytesFromLocationAndReset(0xc398, 4));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x55d7, 4));
				ms.WriteByte(0x73);
				ms.Write(bin.ReadBytesFromLocationAndReset(0x80e8, 9));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x501f, 3));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x80d8, 0xd));
				ms.Write(bin.ReadBytesFromLocationAndReset(0x7260, 4));
				ms.WriteByte(0x49);
				ms.Write(bin.ReadBytesFromLocationAndReset(0xad35, 7));
				patcher.ReplacePartialCommand(0x8075, 0xab, 0x80d1, 0x33, ms.CopyToByteArrayAndDispose());
			}

			// Space/Mirage -> space/mirage
			bin.Position = 0x13843;
			bin.WriteUInt8(0x73);
			bin.Position = 0x1384e;
			bin.WriteUInt8(0x6d);

			// expert angler -> Master Fisher
			bin.Position = 0x1b5ee;
			bin.Write(new byte[] {0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x01, 0x46, 0x69, 0x73, 0x68});
			bin.Position = 0x1d1f9;
			bin.Write(new byte[] {0x4d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x46, 0x69, 0x73, 0x68});

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r4200.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xd6dcc55f71cf2e61ul, 0x93c6a33b53b8879cul, 0x0d4d5958u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r4200.dat", file) };
		}
	}
}
