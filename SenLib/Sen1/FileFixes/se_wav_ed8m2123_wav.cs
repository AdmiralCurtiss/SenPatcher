using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m2123_wav : FileFixBase {
		public override string GetSha1() {
			return "684cc74b0837ff1408124f8b8a05cfd9c9a09195";
		}

		public override string GetSubPath() {
			return @"data/se/wav/ed8" + '\uff4d' + "2123.wav";
		}

		public override string GetSubTargetPath() {
			return @"data/se/wav/ed8m2123.wav";
		}

		public override string GetBackupSubPath() {
			// avoid the fullwidth 'm' in the target path, just in case
			return GetSubTargetPath().Replace('/', '_').Replace('\\', '_');
		}

		public override bool TargetFileExists() {
			return false;
		}

		protected override void DoApply(Stream bin) {
			// actually nothing to do here, we just want to copy the file to a different location
			// we do this because the existing file in the game assets uses a fullwidth lowercase 'm' but the se table asks for a regular ASCII 'm'
			// note that I'm unsure if this is actually required, as I couldn't locate the usage of this sound effect, but might as well...
		}

		public override string GetDescription() {
			return "Fix incorrect filename of ed8m2123.wav -- unknown if this actually affects anything though.";
		}
	}
}
