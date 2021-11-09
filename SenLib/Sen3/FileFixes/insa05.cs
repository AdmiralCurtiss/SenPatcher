using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class insa05 : FileMod {
		public string GetDescription() {
			return "Fix incorrect cut in Chapter 2 map travel sequence.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6c33cf8b1cf93950ul, 0xd802f5ac9de1b213ul, 0xc0b45033u));
			if (file == null) {
				return null;
			}

			var ms = new System.IO.MemoryStream();
			HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(file, new DuplicatableByteArrayStream(Properties.Resources.insa_05), ms);
			ms.Position = 0;
			return new FileModResult[] { new FileModResult("data/movie_us/webm/insa_05.webm", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x6c33cf8b1cf93950ul, 0xd802f5ac9de1b213ul, 0xc0b45033u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/movie_us/webm/insa_05.webm", file) };
		}
	}
}
