using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0800_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in southern Nord plains.";
		}

		public override string GetSha1() {
			return "48d8e5ebfacb29fed7c5c4e75c84e108277cb5fe";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/r0800.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0xf877, 0x46, 0xf89c, 1);
		}
	}
}
