using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_scena_dat_us_t1001_dat : FileMod {
		Sen2Version Version;

		public scripts_scena_dat_us_t1001_dat(Sen2Version version) {
			Version = version;
		}

		public string GetDescription() {
			return "Trista main map data patch for v1.4 (Humble).";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			return TryRevert(storage);
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(
				Version == Sen2Version.v14
				? new HyoutaUtils.Checksum.SHA1(0xace845b437df94fbul, 0xfe2d638a2ec162b4ul, 0x92a657b3u)
				: new HyoutaUtils.Checksum.SHA1(0xfae1d23cd07aa0c9ul, 0x90ca63607e64fcddul, 0xd60a80dau)
			);
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_us/t1001.dat", s) };
		}
	}
}
