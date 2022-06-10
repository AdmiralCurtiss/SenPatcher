using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen3 {
	public static partial class Sen3ExecutablePatches {
		public static void PatchDisableMouseCapture(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;

			// change functions that capture the mouse cursor to not do that
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x140320b38 : 0x14032a048);
			bin.WriteUInt8(0xeb); // jz -> jmp
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x140320d05 : 0x14032a215);
			bin.WriteUInt8(0xeb); // jz -> jmp

			// change function that handles camera movement to not react to mouse movement
			// and not to fall back to WASD camera movement either (legacy code...?)
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400f7be8 : 0x1400f8268);
				branch.WriteJump5Byte(0xe9);
				bin.WriteUInt8(0x90); // nop
				branch.SetTarget(jp ? 0x1400f7d61u : 0x1400f83e1u);
			}

			// skip mouse movement processing function or something like that?
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400f88bd : 0x1400f8f3d);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
		}

		public static void PatchShowMouseCursor(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;

			// remove call to ShowCursor(0)
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x140599444 : 0x1405a5804);
			for (int i = 0; i < 8; ++i) {
				bin.WriteUInt8(0x90); // nop
			}
		}

		public static void PatchDisablePauseOnFocusLoss(Stream bin, Sen3ExecutablePatchState state) {
			bool jp = state.IsJp;

			// 0x1400f6820 -> game active setter
			// 0x1400f7b10 -> game active getter
			// 0x1400f76a4 -> game loop skip when inactive

			// don't silence audio output when unfocused
			using (var branch = new BranchHelper1Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400f943b : 0x1400f9abb);
				branch.WriteJump(0xeb);
				branch.SetTarget(jp ? 0x1400f9474u : 0x1400f9af4u);
			}

			// still run main game loop when unfocused
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1400f7024 : 0x1400f76a4);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop

			// avoid processing mouse clicks when unfocused
			// (this previously happened only implicitly because the game didn't run...)
			// carve out some now-unused code space
			long codespaceStart = (jp ? 0x1400f943b : 0x1400f9abb) + 2;
			long codespaceEnd = jp ? 0x1400f9474 : 0x1400f9af4;
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

				back_to_function.SetTarget((jp ? 0x14012dfa2u : 0x140131522u) + 6);
				bin.Position = state.Mapper.MapRamToRom((jp ? 0x14012dfa2 : 0x140131522) + 2);
				long GetKeyStateAddress = bin.ReadUInt32(le) + ((jp ? 0x14012dfa2 : 0x140131522) + 6);
				long GameStateAddress = jp ? 0x141d43ea0 : 0x141d5a040;

				bin.Position = state.Mapper.MapRamToRom(jp ? 0x14012dfa2 : 0x140131522);
				jump_to_codespace.WriteJump5Byte(0xe9);    // jmp jump_to_codespace
				bin.WriteUInt8(0x90);                      // nop

				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_codespace.SetTarget((ulong)codespace.Address);
				bin.WriteUInt24(0x488d05u, be);                                // lea rax,[static_address_that_holds_game_state]
				SenUtils.WriteRelativeAddress32(GameStateAddress, state.Mapper, bin);
				bin.WriteUInt24(0x488b00u, be);                                // mov rax,[rax]
				bin.WriteUInt24(0x4885c0u, be);                                // test rax,rax
				skip_processing.WriteJump(0x74);                               // jz skip_processing
				bin.WriteUInt56(0x0fb680e01c0000u, be);                        // movzx eax,byte ptr[rax+1ce0h]
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
