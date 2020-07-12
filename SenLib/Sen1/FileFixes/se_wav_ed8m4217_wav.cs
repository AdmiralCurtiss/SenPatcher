using SenLib.Properties;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class se_wav_ed8m4217_wav : FileFromScratchBase {
		public override string GetSubTargetPath() {
			return @"data/se/wav/ed8m4217.wav";
		}

		public override string GetTargetSha1() {
			return "0056ff921028d42f226e25555dd4833619a8cbc8";
		}

		protected override Stream DoCreate() {
			return DecompressHelper.DecompressFromBuffer(Resources.ed8m4217);
		}

		public override string GetDescription() {
			return "Fix missing sound effect in chapter 4. (Lute)";
		}
	}
}
