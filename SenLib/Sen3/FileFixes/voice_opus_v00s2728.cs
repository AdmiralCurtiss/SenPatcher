using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3.FileFixes {
	public class voice_opus_v00s2728 : FileMod {
		public string GetDescription() {
			return "Fix incorrect voice clips for Rean's Ogre S-Crafts.";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var voiceclip27 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0xac35607991b8d7b2ul, 0xe1e1b9cbd4deb578ul, 0x45697915u));
			var voiceclip28 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x4d7ba4fffd80658cul, 0x9db5b475cac0033dul, 0xf4f00d96u));
			if (voiceclip27 == null || voiceclip28 == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/voice_us/opus/v00_s0027.opus", voiceclip27),
				new FileModResult("data/voice_us/opus/v00_s0028.opus", voiceclip28),
			};
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voiceclip27 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x28b5f314748c04e6ul, 0x0a250a6147ed3b8cul, 0x7b3694c8u));
			var voiceclip28 = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x2efce24a4badc67bul, 0x00111edf1a0bf398ul, 0x5c1f26aau));
			if (voiceclip27 == null || voiceclip28 == null) {
				return null;
			}
			return new FileModResult[] {
				new FileModResult("data/voice_us/opus/v00_s0027.opus", voiceclip27),
				new FileModResult("data/voice_us/opus/v00_s0028.opus", voiceclip28),
			};
		}
	}
}
