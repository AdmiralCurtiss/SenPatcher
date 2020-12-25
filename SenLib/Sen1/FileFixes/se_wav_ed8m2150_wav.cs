using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m2150_wav : FileMod {
		public string GetDescription() {
			return "Fix missing sound effect in chapter 6. (Courageous)";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x1d7d909f29ac458eul, 0xf6ffc94cbd0a0a72ul, 0x49c25fdeu));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m2150.wav", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m2150.wav", null) };
		}
	}
}
