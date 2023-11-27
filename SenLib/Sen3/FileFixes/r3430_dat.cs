using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class r3430_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Languedoc Canyon - North.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3efbd8764d61274aul, 0x8750342972e75143ul, 0x131d7721u));
			if (file == null) {
				return null;
			}

			var bin = file.CopyToMemory();
			var patcher = new SenScriptPatcher(bin);

			// A-rank bracer -> A-rank level bracer
			using (var ms = new MemoryStream()) {
				bin.SwapBytes(0x17c53, 0x17c60);
				byte[] data = bin.ReadBytesFromLocationAndReset(0x17c2f, 0x54);
				ms.Write(data, 0, 0x3c);
				ms.Write(new byte[] {0x6c, 0x65, 0x76, 0x65, 0x6c});
				ms.Write(data, 0x3b, 0x16);
				ms.Write(data, 0x52, 2);
				patcher.ReplaceCommand(0x17c2f, 0x54, ms.CopyToByteArrayAndDispose());
			}

			// Naval Fortress -> naval fortress
			bin.Position = 0x23d2c;
			bin.WriteUInt8(0x6e);
			bin.Position = 0x23d32;
			bin.WriteUInt8(0x66);

			// remove space before !
			patcher.RemovePartialCommand(0xff94, 0x29, 0xffb9, 0x1);

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3430.dat", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3efbd8764d61274aul, 0x8750342972e75143ul, 0x131d7721u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/r3430.dat", file) };
		}
	}
}
