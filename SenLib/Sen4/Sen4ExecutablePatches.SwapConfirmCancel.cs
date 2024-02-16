using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static partial class Sen4ExecutablePatches {
		public static void PatchSeparateSwapConfirmCancelOption(Stream bin, Sen4ExecutablePatchState state, bool defaultSwapConfirmCancelOptionOn) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// inject at a call to the ParseBoolean function
			// 1.2.0
			//long injectpos = state.Mapper.MapRamToRom(jp ? 0x1405fa0cb : 0x1405fc65b);
			//long isSwitchButtonFuncPos = state.Mapper.MapRamToRom(jp ? 0x1405fa3d0 : 0x1405fc960);

			// 1.2.1
			long injectpos = state.Mapper.MapRamToRom(jp ? 0x1405fa4ab : 0x1405fca2b);
			long isSwitchButtonFuncPos = state.Mapper.MapRamToRom(jp ? 0x1405fa7b0 : 0x1405fcd30);

			bin.Position = injectpos + 1;
			long parseBooleanRelative = bin.ReadInt32();
			long parseBooleanAbsolute = state.Mapper.MapRomToRam(bin.Position + parseBooleanRelative);

			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var parseBoolean = new BranchHelper4Byte(bin, state.Mapper)) {
				parseBoolean.SetTarget((ulong)parseBooleanAbsolute);

				bin.Position = injectpos;
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)(injectpos + 5)));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				bin.WriteAsciiNullterm("SwapOX");
				long newCodeStartRom = bin.Position;
				long newCodeStartRam = state.Mapper.MapRomToRam(newCodeStartRom);
				jumpToNewCode.SetTarget((ulong)newCodeStartRam);
				parseBoolean.WriteJump5Byte(0xe8); // call parseBoolean

				// config struct byte at 0x4e is unused, so take that to store the setting
				bin.WriteUInt32(0x4c8d464e, be);   // lea r8,[rsi + 4Eh]
				bin.WriteUInt32(0x41c60000 | (defaultSwapConfirmCancelOptionOn ? 1u : 0u), be);   // mov byte ptr[r8],defaultSwapConfirmCancelOptionOn
				bin.WriteUInt24(0x488d15, be);     // lea rdx,[SwapOX]
				bin.WriteInt32((int)(newRegionStartRom - (bin.Position + 4)));
				bin.WriteUInt24(0x498bcf, be);     // mov rcx,r15
				parseBoolean.WriteJump5Byte(0xe8); // call parseBoolean

				jumpBack.WriteJump5Byte(0xe9);     // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Separate Confirm/Cancel swap option");
			}

			// replace function that usually checks for (mapping == Switch) with (SwapOX && mapping != PC)
			using (var dontSwap = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = isSwitchButtonFuncPos;
				bin.WriteUInt16(0x33c0, be);     // xor   eax,eax
				bin.WriteUInt24(0x3a414e, be);   // cmp   al,byte ptr [rcx+4Eh]
				dontSwap.WriteJump(0x74);        // je    dontSwap
				bin.WriteUInt32(0x83794002, be); // cmp   dword ptr [rcx+40h],2
				bin.WriteUInt24(0x0f95c0, be);   // setne al
				dontSwap.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt8(0xc3);            // ret
			}

			return;
		}
	}
}
