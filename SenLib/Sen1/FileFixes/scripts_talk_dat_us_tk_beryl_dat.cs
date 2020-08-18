using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_beryl_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong elipsis in conversation with Beryl.";
		}

		public override string GetSha1() {
			return "af8f356c80083c028824fd41332b003fec834cb1";
		}

		public override string GetSubPath() {
			return "data/scripts/talk/dat_us/tk_beryl.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x5b86, 0x3d, 0x5bb8, 3, new byte[] { 0x27 });
		}
	}
}
