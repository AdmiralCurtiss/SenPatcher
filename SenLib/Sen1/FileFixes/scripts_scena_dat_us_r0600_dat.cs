using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_r0600_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong elipsis in Chapter 6 Jusis bonding event.";
		}

		public override string GetSha1() {
			return "65044a35a4c042fabc4a5a66fd23b0cd8163dfdb";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/r0600.dat";
		}

		protected override void DoApply(Stream bin) {
			bin.Position = 0xc171;
			bin.WriteUInt24(0x2e2e2e);
		}
	}
}
