using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchRemoveBattleAnalyzeDebugScreenshot(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// this appears to be some kind of debug/dev code to screenshot the framebuffer on the monster analysis screen
			// unfortunately it softlocks the game if triggered and the outpath (hardcoded to the z: drive) is not writable,
			// so just jump around this entire block and don't even pretend it's useful...
			uint begin = jp ? 0x4d1479u : 0x4d17a9u;
			uint end = jp ? 0x4d154fu : 0x4d187fu;
			bin.Position = state.Mapper.MapRamToRom(begin);
			for (uint i = begin; i < end; ++i) {
				bin.WriteUInt8(0x90); // nop out the code in case some branch still exists into it -- i don't think so but y'know...
			}
			bin.Position = state.Mapper.MapRamToRom(begin);
			using (BranchHelper4Byte jmp = new BranchHelper4Byte(bin, state.Mapper)) {
				jmp.WriteJump5Byte(0xe9); // jmp
				jmp.SetTarget(end - 2); // past this mess
			}
			bin.Position = state.Mapper.MapRamToRom(end - 2);
			bin.WriteUInt16(0x31c0, be); // xor eax,eax
		}
	}
}
