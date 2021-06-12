using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3.FileFixes {
	public class voice_opus_v00e0441 : FileMod {
		public string GetDescription() {
			return "Fix incorrect voice clip in chapter 2 cutscene ('Inspector').";
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var voiceclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x9c78d23b2b80fa5aul, 0x0042caf00dc67244ul, 0xd6265d25u));
			if (voiceclip == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/voice_us/opus/v00_e0441.opus", voiceclip) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var voiceclip = storage.TryGetDuplicate(new HyoutaUtils.Checksum.SHA1(0x07fb5a4afb6719fful, 0x611f4641e90ad652ul, 0x23d0fae3u));
			if (voiceclip == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult("data/voice_us/opus/v00_e0441.opus", voiceclip) };
		}
	}
}
