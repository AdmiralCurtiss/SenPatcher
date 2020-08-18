using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_c0110_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong apostrophe in Le Sage scene (?)";
		}

		public override string GetSha1() {
			return "a740904f6ad37411825565981daa5f5915b55b69";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/c0110.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0xb2f, 0x61, 0xb34, 3, new byte[] { 0x27 });
		}
	}
}
