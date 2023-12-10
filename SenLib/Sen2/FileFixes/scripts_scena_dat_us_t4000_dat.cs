using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	class scripts_scena_dat_us_t4000_dat : FileMod {
		public string GetDescription() {
			return "Fix typo in Ymir.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8b39f1bd64e65c40ul, 0x194b616b4c43f5d9ul, 0xc4d14c73u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// extra comma
			// NPC in Act 1 Part 1, before departing for Celdic.
			patcher.RemovePartialCommand(0x1215f, 0x99, 0x121a1, 0x1);

			// PS4 changed two 'someday' to 'some day' but as far as I can tell the former is fine for an unspecified time point?
			//using (MemoryStream ms = new MemoryStream()) {
			//	bin.Position = 0x15989;
			//	ms.Write(bin.ReadBytes(0xe0));
			//	bin.DiscardBytes(1);
			//	ms.Write(bin.ReadBytes(0x77));
			//	ms.WriteByte(0x20);
			//	ms.Write(bin.ReadBytes(0x10));
			//	patcher.ReplaceCommand(0x15989, 0x168, ms.CopyToByteArrayAndDispose());
			//}

			// I know this is a lot of ask -> I know this is a lot to ask
			// Patiry/Kargo sidequest in Act 2 Part 1
			bin.Position = 0x43206;
			bin.Write(new byte[] { 0x74, 0x6f });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4000.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x8b39f1bd64e65c40ul, 0x194b616b4c43f5d9ul, 0xc4d14c73u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t4000.dat", file) };
		}
	}
}
