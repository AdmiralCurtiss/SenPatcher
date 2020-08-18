using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0032_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix wrong elipsis in teahouse festival events.";
		}

		public override string GetSha1() {
			return "e7854ac057166d50d94c340ec39403d26173ff9f";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0032.dat";
		}

		protected override void DoApply(Stream bin) {
			bin.Position = 0x2360;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0xe22a;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x11fde;
			bin.WriteUInt24(0x2e2e2e);
			bin.Position = 0x137db;
			bin.WriteUInt24(0x2e2e2e);
		}
	}
}
