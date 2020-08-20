using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t0031_dat : FileFixBase {
		public override string GetDescription() {
			return "Fix typos in Stella Garden.";
		}

		public override string GetSha1() {
			return "66a22b79517c7214b00b2a7a4ac898bc5f231fd8";
		}

		public override string GetSubPath() {
			return "data/scripts/scena/dat_us/t0031.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);

			// double spaces in various lines
			patcher.RemovePartialCommand(0xb5be, 0x3f, 0xb5e2, 1);
			patcher.RemovePartialCommand(0xeb28, 0x49, 0xeb39, 1);
			patcher.RemovePartialCommand(0xeef6, 0xc0, 0xef20, 1);

			// broken alisa line (missing space/linebreak)
			bin.Position = 0x1f7a;
			bin.WriteUInt8(0x01);
			patcher.ExtendPartialCommand(0x1f5c, 0x3c, 0x1f83, new byte[] { 0x20 });
		}
	}
}
