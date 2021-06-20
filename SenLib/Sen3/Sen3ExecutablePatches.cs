using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public static partial class Sen3ExecutablePatches {
		public static void AllowSwitchToNightmare(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;
			EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
			EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;

			// allow nightmare to be selected when difficulty is currently not nightmare
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x14042a398 : 0x140436088);
			bin.WriteUInt32(5, le);

			// don't show warning when trying to switch away from nightmare
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1402360e9 : 0x14023c62e);
			bin.WriteUInt32(0x1e4, le);
		}

		public static void SwapBrokenMasterQuartzValuesForDisplay(Stream bin, Sen3ExecutablePatchState state) {
			if (state.IsJp) return; // JP doesn't have this bug, so nothing to fix
			var be = EndianUtils.Endianness.BigEndian;

			// on function entry:
			// r8w -> 0, 1, or 2 depending on which part of the description to print
			// dx  -> master quartz ID
			// we need to swap when:
			// description is 1 (applies to all of these) and
			// ID is any of 0x0c85, 0x0c86, 0x0c8a, 0x0c8c, 0x0c91, 0x0c95, 0x0c98
			// additionally, on french text only when ID is 0x0c81
			// then swap xmm3 and xmm1 for the call to snprintf_s()

			// ...actually this kinda sucks to hardcode, so instead...
			// we insert a sentinel character at the start of the actual text description in the t_mstqrt file
			// before the sprintf call, we check if this sentinel character is there
			// if yes we swap xmm3 and xmm1, and advance string ptr by 1

			// stack from rsp+31h to rsp+37h looks unused, so stash our sentinel check flag in there... rsp+34h looks good


			// first initialize sentinel check flag on stack near start of function
			using (var jump_to_inject = new BranchHelper4Byte(bin, state.Mapper))
			using (var return_to_function = new BranchHelper4Byte(bin, state.Mapper))
			using (var replaced_call = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(0x14027fce4);
				var call = SenUtils.ReadJump5Byte_x64(bin, state.Mapper);
				replaced_call.SetTarget((ulong)call.address);
				return_to_function.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				bin.Position -= 5;
				jump_to_inject.WriteJump5Byte(0xe9);

				var codespace = state.RegionScriptCompilerFunction;
				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_inject.SetTarget((ulong)codespace.Address);

				replaced_call.WriteJump5Byte(call.jmpbyte);
				bin.WriteUInt40(0xc644243400, be); // mov byte ptr [rsp+34h],0
				return_to_function.WriteJump5Byte(0xe9);

				codespace.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Master Quartz description: Initialization");
			}

			// on strncat() call that appends the description string, check for the sentinel; if it's there skip it and set the flag on the stack
			using (var jump_to_inject = new BranchHelper4Byte(bin, state.Mapper))
			using (var return_to_function = new BranchHelper4Byte(bin, state.Mapper))
			using (var return_to_function_short = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(0x1402800ed);
				ulong instr1 = bin.ReadUInt48(be);
				ulong instr2 = bin.ReadUInt40(be);
				bin.Position -= 6;
				return_to_function.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				bin.WriteUInt48(instr1, be);
				bin.Position -= 11;
				jump_to_inject.WriteJump5Byte(0xe9);

				var codespace = state.RegionScriptCompilerFunction;
				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_inject.SetTarget((ulong)codespace.Address);

				bin.WriteUInt40(instr2, be);
				// rdx contains the address of the string, r8d is free to use (will be overwritten after jmp return_to_function)
				bin.WriteUInt24(0x448a02, be);             // mov r8b,byte ptr[rdx]
				bin.WriteUInt32(0x4180f824, be);           // cmp r8b,24h
				return_to_function_short.WriteJump(0x75);  // jne return_to_function_short
				bin.WriteUInt40(0xc644243401, be);         // mov byte ptr [rsp+34h],1
				bin.WriteUInt24(0x48ffc2, be);             // inc rdx
				return_to_function_short.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				return_to_function.WriteJump5Byte(0xe9);

				codespace.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Master Quartz description: Check sentinel and set flag on stack");
			}

			// check sentinel flag and fix up parameters if it's set
			using (var jump_to_inject = new BranchHelper4Byte(bin, state.Mapper))
			using (var return_to_function = new BranchHelper4Byte(bin, state.Mapper))
			using (var exit_without_swap = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(0x1402801aa);
				ulong overwrittenInstruction = bin.PeekUInt40(be);
				jump_to_inject.WriteJump5Byte(0xe9);
				return_to_function.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));

				var codespace = state.RegionScriptCompilerFunction;
				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_inject.SetTarget((ulong)codespace.Address);

				bin.WriteUInt40(0x448a4c2434, be);  // mov r9b,byte ptr[rsp+34h]
				bin.WriteUInt24(0x4584c9, be);      // test r9b,r9b
				exit_without_swap.WriteJump(0x74);  // jz exit_without_swap
				bin.WriteUInt32(0xf30f7ed3, be);    // movq xmm2,xmm3
				bin.WriteUInt32(0xf30f7ed9, be);    // movq xmm3,xmm1
				bin.WriteUInt32(0xf30f7eca, be);    // movq xmm1,xmm2
				exit_without_swap.SetTarget((ulong)state.Mapper.MapRomToRam(bin.Position));
				bin.WriteUInt40(overwrittenInstruction, be);
				return_to_function.WriteJump5Byte(0xe9);

				codespace.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Master Quartz description: Swap params on sentinel flag");
			}
		}
	}
}
