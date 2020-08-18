using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1500_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong apostrophe in Celdic.";
		}

		public override string GetSha1() {
			return "4f2a0d4b4c1602bbd9aec8dc0785f334c6c285f9";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t1500.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.ReplacePartialCommand(0x11129, 0x34, 0x11136, 3, new byte[] { 0x27 });
		}
	}
}
