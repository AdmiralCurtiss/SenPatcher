using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1010_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix double space when receiving Chapter 6 Free Day quests.";
		}

		public override string GetSha1() {
			return "44c4abb3f8e01dde0e36ca1d11cd433f37c10788";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t1010.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);
			patcher.RemovePartialCommand(0x25f91, 0x13f, 0x2606b, 1);
		}
	}
}
