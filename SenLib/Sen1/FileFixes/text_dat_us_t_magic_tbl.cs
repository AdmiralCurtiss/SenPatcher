using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class text_dat_us_t_magic_tbl : FileFixBase {
		public override string GetSha1() {
			return "d5f7bf4c4c575efd5699e8bbd4040b81276a7284";
		}

		public override string GetSubPath() {
			return @"data/text/dat_us/t_magic.tbl";
		}

		protected override void DoApply(Stream bin) {
			// fix typo in Emma's S-Craft
			bin.SwapBytes(0x3c3a, 0x3c3c);
		}

		public override string GetDescription() {
			return "Fix typo in Emma's S-Craft.";
		}
	}
}
