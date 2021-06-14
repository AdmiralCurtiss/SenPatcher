using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class I_JMP009_pkg : FileMod {
		public string GetDescription() {
			return "Fix name of Eisengard Mountain Range on world map.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x05f162efc3d880e9ul, 0x4398ae4e10108aa0ul, 0x18f30feeu));
			if (file == null) {
				return null;
			}

			var result = I_CVIS0061_pkg.PatchSingleTexturePkg(file, DecompressHelper.DecompressFromBuffer(Properties.Resources.jump009));
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_JMP009.pkg", result) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x05f162efc3d880e9ul, 0x4398ae4e10108aa0ul, 0x18f30feeu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_JMP009.pkg", file) };
		}
	}
}
