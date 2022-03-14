using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_scena_asm_t1001_tbl : FileMod {
		Sen2Version Version;

		public scripts_scena_asm_t1001_tbl(Sen2Version version) {
			Version = version;
		}

		public string GetDescription() {
			return "Trista main map data patch for v1.4 (Humble).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			return TryRevert(storage);
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			if (Version == Sen2Version.v14) {
				return new FileModResult[] {
					new FileModResult("data/scripts/scena/asm/t1001.tbl", null),
					new FileModResult("data/scripts/scena/asm", FileModResultType.DeleteFolder)
				};
			}

			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x568a1ae375a6077eul, 0xf5c6fb8e277a333ful, 0x1979505bu));
			if (s == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/scripts/scena/asm", FileModResultType.CreateFolder),
				new FileModResult("data/scripts/scena/asm/t1001.tbl", s)
			};
		}
	}
}
