using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_talk_dat_us_tk_heinrich_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong elipsis in conversation with Heinrich.";
		}

		public override string GetSha1() {
			return "491c365d592bb90029e7543d893d47bd5e66139d";
		}

		public override string GetSubPath() {
			return "data/scripts/talk/dat_us/tk_heinrich.dat";
		}

		protected override void DoApply(Stream bin) {
			bin.Position = 0x1168;
			bin.WriteUInt24(0x2e2e2e);
		}
	}
}
