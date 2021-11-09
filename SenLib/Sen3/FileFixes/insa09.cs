using HyoutaUtils;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3.FileFixes {
	class insa09 : FileMod {
		public string GetDescription() {
			return "Fix duplicate Chapter 3 map travel sequence.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x97b37cb0d324014dul, 0x8db6b2965f1836eful, 0xfdb1ce01u));
			if (file == null) {
				return null;
			}

			var ms = new System.IO.MemoryStream();
			HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(file, new DuplicatableByteArrayStream(Properties.Resources.insa_09), ms);
			ms.Position = 0;
			return new FileModResult[] { new FileModResult("data/movie_us/webm/insa_09.webm", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var file = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x97b37cb0d324014dul, 0x8db6b2965f1836eful, 0xfdb1ce01u));
			if (file == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/movie_us/webm/insa_09.webm", file) };
		}
	}
}
