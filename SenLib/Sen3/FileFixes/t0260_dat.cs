using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0260_dat : FileMod {
		public string GetDescription() {
			return "Fix Hamilton gender (scene 1, text only).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf00fc1a818c84469ul, 0xfd34cfb593d03ad4ul, 0x24393aceu));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// switch gender in line from Tita
			{
				MemoryStream ms = new MemoryStream();
				MemoryStream old = new PartialStream(file, 0x122c7, 0xcb).CopyToMemoryAndDispose();
				old.Position = 0;
				StreamUtils.CopyStream(old, ms, 0xf);
				ms.WriteUInt8(0x73);
				old.Position = 0xf;
				StreamUtils.CopyStream(old, ms, 0x75);
				ms.WriteUInt8(0x65);
				ms.WriteUInt8(0x72);
				old.Position = 0x86;
				StreamUtils.CopyStream(old, ms, 0xe);
				ms.WriteUInt8(0x73);
				old.Position = 0x94;
				StreamUtils.CopyStream(old, ms, 0x37);
				patcher.ReplaceCommand(0x122c7, 0xcb, ms.CopyToByteArrayAndDispose());
			}

			// switch gender in line from George
			{
				MemoryStream ms = new MemoryStream();
				MemoryStream old = new PartialStream(file, 0x123cb, 0x92).CopyToMemoryAndDispose();
				old.Position = 0;
				StreamUtils.CopyStream(old, ms, 0x33);
				ms.WriteUInt8(0x65);
				ms.WriteUInt8(0x72);
				old.Position = 0x35;
				StreamUtils.CopyStream(old, ms, 0x19);
				ms.WriteUInt8(0x73);
				old.Position = 0x4e;
				StreamUtils.CopyStream(old, ms, 0x17);
				ms.WriteUInt8(0x65);
				ms.WriteUInt8(0x72);
				old.Position = 0x67;
				StreamUtils.CopyStream(old, ms, 0x2b);
				patcher.ReplaceCommand(0x123cb, 0x92, ms.CopyToByteArrayAndDispose());
			}

			// switch gender in line from Schmidt
			patcher.ExtendPartialCommand(0x1380a, 0x73, 0x1386e, new byte[] { 0x73 });

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0260.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xf00fc1a818c84469ul, 0xfd34cfb593d03ad4ul, 0x24393aceu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0260.dat", file) };
		}
	}
}
