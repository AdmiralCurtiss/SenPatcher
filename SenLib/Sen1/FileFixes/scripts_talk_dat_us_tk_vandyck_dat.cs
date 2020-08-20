using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_vandyck_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in conversation with Vandyck.";
		}

		public override string GetSha1() {
			return "b838141d25f707a7c95191db2f8c324a3e0a34c0";
		}

		public override string GetSubPath() {
			return "data/scripts/talk/dat_us/tk_vandyck.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1312, 0x183, 0x1420, 1);
		}
	}
}
