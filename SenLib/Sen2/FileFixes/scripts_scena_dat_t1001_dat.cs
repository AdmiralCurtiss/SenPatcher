using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2.FileFixes {
	public class scripts_scena_dat_t1001_dat : FileMod {
		Sen2Version Version;

		public scripts_scena_dat_t1001_dat(Sen2Version version) {
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
				? new HyoutaUtils.Checksum.SHA1(0x3d75d79e3201f8f5ul, 0xac61c206f8cc86dbul, 0x7c4651ddu)
				: new HyoutaUtils.Checksum.SHA1(0x24b90bc222efb431ul, 0xa05941973b3bcbd7ul, 0xe3599d81u)
			);
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat/t1001.dat", s) };
		}
	}
}
