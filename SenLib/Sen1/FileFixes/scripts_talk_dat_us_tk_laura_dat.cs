using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_laura_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in conversation with Laura.";
		}

		public override string GetSha1() {
			return "f423fb1dfddde29d3e26a40ceed87982b899cdca";
		}

		public override string GetSubPath() {
			return "data/scripts/talk/dat_us/tk_laura.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1881, 0x73, 0x18a7, 1);
		}
	}
}
