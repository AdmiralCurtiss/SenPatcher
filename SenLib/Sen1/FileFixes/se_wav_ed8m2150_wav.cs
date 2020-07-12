using SenLib.Properties;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m2150_wav : FileFromScratchBase {
		public override string GetSubTargetPath() {
			return @"data/se/wav/ed8m2150.wav";
		}

		public override string GetTargetSha1() {
			return "1d7d909f29ac458ef6ffc94cbd0a0a7249c25fde";
		}

		protected override Stream DoCreate() {
			return DecompressHelper.DecompressFromBuffer(Resources.ed8m2150);
		}

		public override string GetDescription() {
			return "Fix missing sound effect in chapter 6. (Courageous)";
		}
	}
}
