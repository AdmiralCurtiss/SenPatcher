using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0080_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space in Student Union building (chess club).";
		}

		public override string GetSha1() {
			return "d5805f2f25de668a4ececc8f6cad0aaae64a3cf8";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0080.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x1f278, 0x88, 0x1f2df, 1);
		}
	}
}
