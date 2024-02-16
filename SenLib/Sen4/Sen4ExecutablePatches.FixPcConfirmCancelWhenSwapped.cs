using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static partial class Sen4ExecutablePatches {
		public static void PatchFixPcConfirmCancelWhenSwapped(Stream bin, Sen4ExecutablePatchState state) {
			bool jp = state.IsJp;
			var be = EndianUtils.Endianness.BigEndian;

			// inject right after the 'id' value is read and stored from the XML

			// 1.2.0
			//long injectpos = state.Mapper.MapRamToRom(jp ? 0x1400e913e : 0x1400eb1fe);
			//long isSwitchButtonFuncPos = jp ? 0x1405fa3d0 : 0x1405fc960;
			//long isDynamicPromptsFuncPos = jp ? 0x1405fa430 : 0x1405fc9c0;
			//long getPcSettingsFuncPos = jp ? 0x1405fa290 : 0x1405fc820;

			// 1.2.1
			long injectpos = state.Mapper.MapRamToRom(jp ? 0x1400e914e : 0x1400eb20e);
			long isSwitchButtonFuncPos = jp ? 0x1405fa7b0 : 0x1405fcd30;
			long isDynamicPromptsFuncPos = jp ? 0x1405fa810 : 0x1405fcd90;
			long getPcSettingsFuncPos = jp ? 0x1405fa670 : 0x1405fcbf0;

			bin.Position = injectpos;
			uint overwrittenInstruction = bin.ReadUInt24(be);
			long strBindingRelative = bin.ReadInt32();
			long strBindingAbsolute = state.Mapper.MapRomToRam(bin.Position) + strBindingRelative;

			using (var jumpToNewCode = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBack = new BranchHelper4Byte(bin, state.Mapper))
			using (var jumpBackShort = new BranchHelper1Byte(bin, state.Mapper))
			using (var check5 = new BranchHelper1Byte(bin, state.Mapper))
			using (var getPcSettings = new BranchHelper4Byte(bin, state.Mapper))
			using (var isSwitchButton = new BranchHelper4Byte(bin, state.Mapper))
			using (var isDynamicPrompts = new BranchHelper4Byte(bin, state.Mapper)) {
				getPcSettings.SetTarget((ulong)getPcSettingsFuncPos);
				isSwitchButton.SetTarget((ulong)isSwitchButtonFuncPos);
				isDynamicPrompts.SetTarget((ulong)isDynamicPromptsFuncPos);

				bin.Position = injectpos;
				bin.WriteUInt8(0x90); // nop
				bin.WriteUInt8(0x90); // nop
				jumpToNewCode.WriteJump5Byte(0xe9); // jmp jumpToNewCode
				jumpBack.SetTarget(state.Mapper.MapRomToRam((ulong)(injectpos + 7)));

				long newRegionStartRam = state.RegionScriptCompilerFunction.Address;
				long newRegionStartRom = state.Mapper.MapRamToRom(newRegionStartRam);
				bin.Position = newRegionStartRom;
				jumpToNewCode.SetTarget((ulong)newRegionStartRam);

				// check if both swapped buttons are enabled and dynamic prompts are disabled
				getPcSettings.WriteJump5Byte(0xe8);    // call getPcSettings
				bin.WriteUInt24(0x488bc8, be);         // mov rcx,rax
				isSwitchButton.WriteJump5Byte(0xe8);   // call isSwitchButton
				bin.WriteUInt16(0x84c0, be);           // test al,al
				jumpBackShort.WriteJump(0x74);         // jz jumpBackShort
				getPcSettings.WriteJump5Byte(0xe8);    // call getPcSettings
				bin.WriteUInt24(0x488bc8, be);         // mov rcx,rax
				isDynamicPrompts.WriteJump5Byte(0xe8); // call isDynamicPrompts
				bin.WriteUInt16(0x84c0, be);           // test al,al
				jumpBackShort.WriteJump(0x75);         // jnz jumpBackShort

				// checks out, so swap around config for 4 and 5
				bin.WriteUInt40(0x488d442438, be);     // lea rax,[rsp+38h]
				bin.WriteUInt16(0x8b08, be);           // mov ecx,dword ptr[rax]
				bin.WriteUInt24(0x83f904, be);         // cmp ecx,4
				check5.WriteJump(0x75);                // jne check5
				bin.WriteUInt48(0xc70005000000, be);   // mov dword ptr[rax],5
				jumpBackShort.WriteJump(0xeb);         // jmp jumpBackShort
				check5.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				bin.WriteUInt24(0x83f905, be);         // cmp ecx,5
				jumpBackShort.WriteJump(0x75);         // jne jumpBackShort
				bin.WriteUInt48(0xc70004000000, be);   // mov dword ptr[rax],4

				jumpBackShort.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				bin.WriteUInt24(overwrittenInstruction, be);
				bin.WriteInt32((int)(strBindingAbsolute - state.Mapper.MapRomToRam(bin.Position + 4)));
				jumpBack.WriteJump5Byte(0xe9);     // jmp jumpBack

				state.RegionScriptCompilerFunction.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Fix PC Confirm/Cancel when non-dynamic and swapped");
			}

			return;
		}
	}
}
