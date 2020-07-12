using SenLib.Properties;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m4097_wav : FileFromScratchBase {
		public override string GetSubTargetPath() {
			return @"data/se/wav/ed8m4097.wav";
		}

		public override string GetTargetSha1() {
			return "e336f59af7a03b6ef6ec2d99383718857b26ecf3";
		}

		protected override Stream DoCreate() {
			return DecompressHelper.DecompressFromBuffer(Resources.ed8m4097);
		}

		public override string GetDescription() {
			return "Fix missing sound effect in chapter 4. (Phantom Thief B)";
		}
	}
}
