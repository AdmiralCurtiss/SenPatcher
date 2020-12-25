using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m2123_wav : FileMod {
		public string GetDescription() {
			return "Fix incorrect filename of ed8m2123.wav -- unknown if this actually affects anything though.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			// just copy the file to a different location
			// we do this because the existing file in the game assets uses a fullwidth lowercase 'm' but the se table asks for a regular ASCII 'm'
			// note that I'm unsure if this is actually required, as I couldn't locate the usage of this sound effect, but might as well...
			var s = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x684cc74b0837ff14ul, 0x08124f8b8a05cfd9ul, 0xc9a09195u));
			if (s == null) {
				return null;
			}
			MemoryStream bin = s.CopyToMemoryAndDispose();
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m2123.wav", bin) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			// when reverting just delete the file again
			return new FileModResult[] { new FileModResult("data/se/wav/ed8m2123.wav", null) };
		}
	}
}
