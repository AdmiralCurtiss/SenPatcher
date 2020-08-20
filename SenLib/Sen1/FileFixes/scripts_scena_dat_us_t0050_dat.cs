using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0050_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in Mishy Panic.";
		}

		public override string GetSha1() {
			return "d363fc2114ec8421c24b47c29a4a2baded31cfb5";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0050.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x11b02, 0x44, 0x11b10, 1);
		}
	}
}
