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
			// fix line from Alisa about the upcoming exams
			// this has been fixed in the PS4 version but for some reason was not patched back into the PC version
			{
				// we're replacing a textbox command with a different one here
				// but since the new textbox command is longer than the old one we need to overwrite it with a jump,
				// write the new command in free space at the bottom of the file, and then jump back
				uint originalLocation = 0x25b3e;
				uint originalLength = 0x38;
				bin.Position = originalLocation;
				byte[] originalCommand = bin.ReadUInt8Array(originalLength);
				uint nextCommandLocation = (uint)bin.Position;

				bin.Position = bin.Length;
				bin.WriteAlign(4);
				uint newCommandLocation = (uint)bin.Position;
				// first 0x1b bytes are the same
				bin.Write(originalCommand, 0, 0x1b);
				// then the changed stuff
				bin.Write(new byte[] { 0x66, 0x65, 0x65, 0x6c, 0x3f, 0x20, 0x41, 0x6c, 0x6c, 0x01, 0x70, 0x72, 0x65, 0x70, 0x70, 0x65, 0x64, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x72, 0x65, 0x61, 0x64, 0x79, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x6f, 0x75, 0x72 });
				// last 0x9 bytes are the same again
				bin.Write(originalCommand, (int)originalLength - 0x9, 0x9);
				// then a jump back to the actual script
				bin.WriteUInt8(0x03);
				bin.WriteUInt32(nextCommandLocation, EndianUtils.Endianness.LittleEndian);

				// jump to the new command back at the original command
				bin.Position = originalLocation;
				bin.WriteUInt8(0x03);
				bin.WriteUInt32(newCommandLocation, EndianUtils.Endianness.LittleEndian);
				// and to be clean, dummy out the rest of the old command
				for (uint i = 0; i < originalLength - 5; ++i) {
					bin.WriteUInt8(0x09); // this should be a nop
				}
			}
		}
	}
}
