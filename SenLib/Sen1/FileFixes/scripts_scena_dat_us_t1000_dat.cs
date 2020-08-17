using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class scripts_scena_dat_us_t1000_dat : FileFixBase {
		public override string GetSha1() {
			return "84d3de50b7318f20b4fe48836404d134a124be52";
		}

		public override string GetSubPath() {
			return @"data/scripts/scena/dat_us/t1000.dat";
		}

		protected override void DoApply(Stream bin) {
			var patcher = new SenScriptPatcher(bin);

			// fix line from Alisa about the upcoming exams
			// this has been fixed in the PS4 version but for some reason was not patched back into the PC version
			{
				uint originalLocation = 0x25b3e;
				uint originalLength = 0x38;
				byte[] originalCommand = bin.ReadBytesFromLocationAndReset(originalLocation, originalLength);

				MemoryStream newCommand = new MemoryStream();
				newCommand.Write(originalCommand, 0, 0x1b);
				newCommand.Write(new byte[] {
					0x66, 0x65, 0x65, 0x6c, 0x3f, 0x20, 0x41, 0x6c, 0x6c, 0x01, 0x70, 0x72, 0x65, 0x70, 0x70, 0x65,
					0x64, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x72, 0x65, 0x61, 0x64, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20,
					0x6f, 0x75, 0x72
				});
				newCommand.Write(originalCommand, (int)originalLength - 0x9, 0x9);

				patcher.ReplaceCommand(originalLocation, originalLength, newCommand.CopyToByteArrayAndDispose());
			}

			// two lines later, linebreak was moved to a nicer spot, might as well apply that too
			bin.SwapBytes(0x25bee, 0x25bfb);
		}

		public override string GetDescription() {
			return "Fix continuity error in chapter 3. (text only; this was officially fixed on PS4, but not on PC)";
		}
	}
}
