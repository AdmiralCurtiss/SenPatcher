using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m4217_wav : FileMod {
		public string GetDescription() {
			return "Fix missing sound effect in chapter 4. (Lute)";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x0056ff921028d42ful, 0x226e25555dd48336ul, 0x19a8cbc8u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m4217.wav", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m4217.wav", null) };
		}
	}
}
