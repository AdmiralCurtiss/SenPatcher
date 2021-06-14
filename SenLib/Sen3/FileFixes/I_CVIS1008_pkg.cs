using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class I_CVIS1008_pkg : FileMod {
		public string GetDescription() {
			return "Fix Rontes nametag.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3721996e74336af0ul, 0x133f096b1f717e83ul, 0xb9bdc075u));
			if (file == null) {
				return null;
			}

			var result = I_CVIS0061_pkg.PatchSingleTexturePkg(file, DecompressHelper.DecompressFromBuffer(Properties.Resources.cvis1008));
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_CVIS1008.pkg", result) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x3721996e74336af0ul, 0x133f096b1f717e83ul, 0xb9bdc075u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/asset/D3D11_us/I_CVIS1008.pkg", file) };
		}
	}
}
