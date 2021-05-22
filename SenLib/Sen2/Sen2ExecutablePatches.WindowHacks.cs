using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchDisableMouseCapture(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;

			// change function that captures the mouse cursor to not do that
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x581b0e : 0x581ece);
			bin.WriteUInt8(0xeb); // jz -> jmp

			// change function that handles camera movement to not react to mouse movement
			// and not to fall back to WASD camera movement either (legacy code...?)
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x4148b8 : 0x4148c8);
				branch.WriteJump5Byte(0xe9);
				bin.WriteUInt8(0x90); // nop
				branch.SetTarget(jp ? 0x414a73u : 0x414a83u);
			}

			// remove call to ShowCursor(0)
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x6cfb3a : 0x6d0afa);
			for (int i = 0; i < 8; ++i) {
				bin.WriteUInt8(0x90); // nop
			}

			// skip mouse movement processing function or something like that?
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x4154d2 : 0x4154e2);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
		}

		public static void PatchDisablePauseOnFocusLoss(Stream bin, Sen2ExecutablePatchState state) {
			bool jp = state.IsJp;

			// change dinput keyboard SetCooperativeLevel from DISCL_NONEXCLUSIVE | DISCL_FOREGROUND to DISCL_NONEXCLUSIVE | DISCL_BACKGROUND
			//bin.Position = state.Mapper.MapRamToRom(jp ? 0 : 0x7faf07);
			//bin.WriteUInt8(0x0a);

			// 0x413660 -> this branch is false when game is inactive, which skips most of the game update logic
			// 0x413A10 -> GameHasFocusCached() or something along those lines?
			// 0x412520 -> corresponding setter to the above
			// 0x4161BE -> called from here
			// 0x415cc0 -> function registered as the window procedure in SetWindowLongA()

			// don't silence audio output when unfocused
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x4161b1 : 0x4161c1);
				branch.WriteJump5Byte(0xe9);
				branch.SetTarget(jp ? 0x4161f9u : 0x416209u);
			}

			// still run main game loop when unfocused
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x413650 : 0x413660);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop

			// avoid processing mouse clicks when unfocused
			// (this previously happened only implicitly because the game didn't run...)
			// carve out some now-unused code space
			uint codespaceStart = jp ? 0x4161b6u : 0x4161c6u;
			uint codespaceEnd = jp ? 0x4161f9u : 0x416209u;
			var codespace = new RegionHelper(codespaceStart, codespaceEnd - codespaceStart, "Pause on Focus Loss: Codespace Region");
			bin.Position = state.Mapper.MapRamToRom(codespace.Address);
			for (uint i = 0; i < codespace.Remaining; ++i) {
				bin.WriteUInt8(0xcc); // int 3
			}

			// and assemble some logic to skip mouse button processing when unfocused
			using (var jump_to_codespace = new BranchHelper4Byte(bin, state.Mapper))
			using (var back_to_function = new BranchHelper4Byte(bin, state.Mapper))
			using (var skip_processing = new BranchHelper1Byte(bin, state.Mapper)) {
				var be = EndianUtils.Endianness.BigEndian;

				back_to_function.SetTarget(jp ? 0x6ad622u : 0x6ae652u);
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x6ad61e : 0x6ae64e);
				ulong GetKeyStateAddress = bin.ReadUInt32(be);
				ulong GameStateAddress = (jp ? 0x116f3c0u : 0x1173c40u).ToEndian(be);

				// CS2 has this copy-pasted 5 times for the 5 supported mouse buttons so inject in all of them...
				for (int i = 0; i < 5; ++i) {
					bin.Position = state.Mapper.MapRamToRom((jp ? 0x6ad61b : 0x6ae64b) + i * 0x11);
					jump_to_codespace.WriteJump5Byte(0xe9);    // jmp jump_to_codespace
					bin.WriteUInt8(0x90);                      // nop
					bin.WriteUInt8(0x90);                      // nop
				}

				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_codespace.SetTarget(codespace.Address);
				bin.WriteUInt48(0x8b0d00000000u | GameStateAddress, be);   // mov ecx,[static_address_that_holds_game_state]
				bin.WriteUInt16(0x85c9, be);                               // test ecx,ecx
				skip_processing.WriteJump(0x74);                           // jz skip_processing
				bin.WriteUInt48(0x8a89c4090000, be);                       // mov cl,byte ptr[ecx+9c4h]   ; cl now holds 0 if unfocused, 1 if focused
				bin.WriteUInt16(0x84c9, be);                               // test cl,cl
				skip_processing.WriteJump(0x74);                           // jz skip_processing
				bin.WriteUInt48(0x8b0d00000000u | GetKeyStateAddress, be); // mov ecx,[USER32.DLL:GetKeyState]
				bin.WriteUInt16(0x85c9, be);                               // test ecx,ecx
				skip_processing.WriteJump(0x74);                           // jz skip_processing
				bin.WriteUInt8(0x50);                                      // push eax
				bin.WriteUInt16(0xffd1, be);                               // call ecx
				back_to_function.WriteJump5Byte(0xe9);                     // jmp back_to_function
				skip_processing.SetTarget(state.Mapper.MapRomToRam((ulong)bin.Position));
				bin.WriteUInt16(0x33c0, be);                               // xor eax,eax
				back_to_function.WriteJump5Byte(0xe9);                     // jmp back_to_function

				codespace.TakeToAddress(state.Mapper.MapRomToRam(bin.Position), "Pause on Focus Loss: don't process mouse clicks");
			}
		}
	}
}
