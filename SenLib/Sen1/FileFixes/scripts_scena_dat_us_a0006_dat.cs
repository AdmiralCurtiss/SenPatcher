using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_a0006_dat : FileFixBase {
		public override string GetDescription() {
			// I think this might actually be a debug map but whatever...
			return "Fix double spaces in a0006. (?)";
		}

		public override string GetSha1() {
			return "73fd289006017eab8816636d998d21aa8fb38d68";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/a0006.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x3a67, 0x6f, 0x3ac4, 1);
			patcher.RemovePartialCommand(0x3e5f, 0x5f, 0x3e8c, 1);
		}
	}
}
