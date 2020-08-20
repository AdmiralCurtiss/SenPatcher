using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_edel_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in conversation with Edel.";
		}

		public override string GetSha1() {
			return "dc5deaa30247aafaf2a369da2268d4082ba310c5";
		}

		public override string GetSubPath() {
			return "data/scripts/talk/dat_us/tk_edel.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x3179, 0x48, 0x31aa, 1);
		}
	}
}
