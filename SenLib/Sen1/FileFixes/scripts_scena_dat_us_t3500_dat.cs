using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t3500_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong elipsis in main Roer area.";
		}

		public override string GetSha1() {
			return "d18f9880c045b969afd8c6a8836ee6e86810aa4e";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t3500.dat";
		}

		protected override void DoApply(Stream bin) {
			bin.Position = 0x9ce6;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x28b20;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x29b60;
			bin.WriteUInt24(0x2e2e2e);
		}
	}
}
