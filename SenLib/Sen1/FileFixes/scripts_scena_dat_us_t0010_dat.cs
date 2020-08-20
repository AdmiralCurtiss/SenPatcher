using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0010_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in Chapter 6 scene in front of school building.";
		}

		public override string GetSha1() {
			return "8a76ff88baf96b5e72e675d0d5d3b75a72cc3989";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0010.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0xfbbf, 0x53, 0xfbcf, 1);
		}
	}
}
