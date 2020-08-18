using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0000_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong apostrophe on Thors campus grounds.";
		}

		public override string GetSha1() {
			return "83fc174bcce22201fe2053f855e8879b3091e649";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0000.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x1c6e0, 0x8b, 0x1c71f, 3, new byte[] { 0x27 });
			patcher.ReplacePartialCommand(0x1c6e0, 0x89, 0x1c751, 3, new byte[] { 0x27 });
		}
	}
}
