using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class t0250_dat : FileMod {
		public string GetDescription() {
			return "Terminology fixes in Pool/Training Ground.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1ba8784695ed8630ul, 0x9b72a3104c7b0b81ul, 0xb67b503bu));
			if (file == null) {
				return null;
			}

			// This file has a TON of changes, which makes patching them in-code very annoying and error-prone.
			// I suspect what happened here is that the PC version accidentally used the on-disc PS4 script instead
			// of the 1.02 patch like all the other files. So to save us some pain we'll just patch it directly.

			var ms = new System.IO.MemoryStream();
			HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(file, DecompressHelper.DecompressFromBuffer(Properties.Resources.t0250_dat).CopyToByteArrayStreamAndDispose(), ms);
			ms.Position = 0;

			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0250.dat", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1ba8784695ed8630ul, 0x9b72a3104c7b0b81ul, 0xb67b503bu));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/scripts/scena/dat_en/t0250.dat", file) };
		}
	}
}
