using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2.FileFixes {
	public class text_dat_us_t_magic_tbl : FileFixBase {
		public override string GetSha1() {
			return "92de0d29c0ad4a9ea935870674976924d5df756d";
		}

		public override string GetSubPath() {
			return @"data/text/dat_us/t_magic.tbl";
		}

		protected override void DoApply(Stream bin) {
			// fix typo in Emma's S-Craft
			{
				bin.Position = 0x642b;
				byte b0 = bin.ReadUInt8();
				byte b1 = bin.ReadUInt8();
				byte b2 = bin.ReadUInt8();
				bin.Position = 0x642b;
				bin.WriteUInt8(b2);
				bin.WriteUInt8(b1);
				bin.WriteUInt8(b0);
			}
		}
	}
}
