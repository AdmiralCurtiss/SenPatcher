using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static partial class Sen4ExecutablePatches {
		public static void PatchDisableMouseCapture(Stream bin, Sen4ExecutablePatchState state) {
			bool jp = state.IsJp;

			// change function that captured the mouse cursor to not do that
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x14030a8ef : 0x14030d3bf);
			bin.WriteUInt8(0xeb); // jz -> jmp

			// change function that handles camera movement to not react to mouse movement
			// and not to fall back to WASD camera movement either (legacy code...?)
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400b1e78 : 0x1400b1e98);
				branch.WriteJump5Byte(0xe9);
				bin.WriteUInt8(0x90); // nop
				branch.SetTarget(jp ? 0x1400b2034u : 0x1400b2054u);
			}

			// skip mouse movement processing function or something like that?
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400b2cd1 : 0x1400b2cf1);
			for (int i = 0; i < 5; ++i) {
				bin.WriteUInt8(0x90); // nop
			}
		}

		public static void PatchShowMouseCursor(Stream bin, Sen4ExecutablePatchState state) {
			bool jp = state.IsJp;

			// remove call to ShowCursor(0)
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1405fa9e4 : 0x1405fcf74);
			for (int i = 0; i < 7; ++i) {
				bin.WriteUInt8(0x90); // nop
			}
			bin.WriteUInt8(0xc3); // ret
		}

		public static void PatchDisablePauseOnFocusLoss(Stream bin, Sen4ExecutablePatchState state) {
			bool jp = state.IsJp;

			// 0x1400b1740 -> game active getter

			// don't silence audio output when unfocused
			using (var branch = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400b3461 : 0x1400b3481);
				branch.WriteJump(0xeb);
				branch.SetTarget(jp ? 0x1400b34a4u : 0x1400b34c4u);
			}

			// still run main game loop when unfocused
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400b12f4 : 0x1400b1314);
			for (int i = 0; i < 6; ++i) {
				bin.WriteUInt8(0x90); // nop
			}

			// avoid processing mouse clicks when unfocused
			// (this previously happened only implicitly because the game didn't run...)
			// carve out some now-unused code space
			long codespaceStart = (jp ? 0x1400b3461 : 0x1400b3481) + 2;
			long codespaceEnd = jp ? 0x1400b34a4 : 0x1400b34c4;
			var codespace = new RegionHelper64(codespaceStart, (uint)(codespaceEnd - codespaceStart), "Pause on Focus Loss: Codespace Region");
			bin.Position = state.Mapper.MapRamToRom(codespace.Address);
			for (uint i = 0; i < codespace.Remaining; ++i) {
				bin.WriteUInt8(0xcc); // int 3
			}

			// and assemble some logic to skip mouse button processing when unfocused
			using (var jump_to_codespace = new BranchHelper4Byte(bin, state.Mapper))
			using (var back_to_function = new BranchHelper4Byte(bin, state.Mapper))
			using (var back_to_function_short = new BranchHelper1Byte(bin, state.Mapper))
			using (var skip_processing = new BranchHelper1Byte(bin, state.Mapper)) {
				var le = EndianUtils.Endianness.LittleEndian;
				var be = EndianUtils.Endianness.BigEndian;

				back_to_function.SetTarget((jp ? 0x1400e9e3cu : 0x1400ebefcu) + 6);
				bin.Position = state.Mapper.MapRamToRom((jp ? 0x1400e9e3c : 0x1400ebefc) + 2);
				long GetKeyStateAddress = bin.ReadUInt32(le) + ((jp ? 0x1400e9e3c : 0x1400ebefc) + 6);
				long GameStateAddress = jp ? 0x143784650 : 0x1437867d0;

				bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400e9e3c : 0x1400ebefc);
				jump_to_codespace.WriteJump5Byte(0xe9);    // jmp jump_to_codespace
				bin.WriteUInt8(0x90);                      // nop

				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_codespace.SetTarget((ulong)codespace.Address);
				bin.WriteUInt24(0x488d05u, be);                                // lea rax,[static_address_that_holds_game_state]
				SenUtils.WriteRelativeAddress32(GameStateAddress, state.Mapper, bin);
				bin.WriteUInt24(0x488b00u, be);                                // mov rax,[rax]
				bin.WriteUInt24(0x4885c0u, be);                                // test rax,rax
				skip_processing.WriteJump(0x74);                               // jz skip_processing
				bin.WriteUInt56(0x0fb680b01c0000u, be);                        // movzx eax,byte ptr[rax+1cb0h]
				bin.WriteUInt16(0x85c0, be);                                   // test eax,eax
				skip_processing.WriteJump(0x74);                               // jz skip_processing
				bin.WriteUInt24(0x488d05u, be);                                // lea rax,[GetKeyState]
				SenUtils.WriteRelativeAddress32(GetKeyStateAddress, state.Mapper, bin);
				bin.WriteUInt24(0x488b00u, be);                                // mov rax,[rax]
				bin.WriteUInt24(0x4885c0u, be);                                // test rax,rax
				skip_processing.WriteJump(0x74);                               // jz skip_processing
				bin.WriteUInt16(0xffd0, be);                                   // call rax
				back_to_function_short.WriteJump(0xeb);                        // jmp back_to_function_short
				skip_processing.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt24(0x6633c0, be);                                 // xor ax,ax
				back_to_function_short.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				back_to_function.WriteJump5Byte(0xe9);                         // jmp back_to_function

				codespace.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Pause on Focus Loss: don't process mouse clicks");
			}
		}
	}
}
