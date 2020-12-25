using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m4097_wav : FileMod {
		public string GetDescription() {
			return "Fix missing sound effect in chapter 4. (Phantom Thief B)";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xe336f59af7a03b6eul, 0xf6ec2d9938371885ul, 0x7b26ecf3u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m4097.wav", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m4097.wav", null) };
		}
	}
}
