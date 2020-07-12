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
			{
				bin.Position = 0x3c3a;
				byte b0 = bin.ReadUInt8();
				byte b1 = bin.ReadUInt8();
				byte b2 = bin.ReadUInt8();
				bin.Position = 0x3c3a;
				bin.WriteUInt8(b2);
				bin.WriteUInt8(b1);
				bin.WriteUInt8(b0);
			}
		}

		public override string GetDescription() {
			return "Fix typo in Emma's S-Craft.";
		}
	}
}
