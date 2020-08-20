using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_c0100_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix duplicate line on Imperial Chronicle sign.";
		}

		public override string GetSha1() {
			return "9a75c8439ca12500437af585d0ae94744b620dfd";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/c0100.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x27aed, 0x2c, 0x27b03, 0x13);
		}
	}
}
