using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static partial class Sen1ExecutablePatches {
		public static void PatchDisableMouseCapture(Stream bin, Sen1ExecutablePatchState state) {
			bool jp = state.IsJp;

			// change function that captures the mouse cursor to not do that
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x5de6c6 : 0x5df536);
			bin.WriteUInt8(0xeb); // jz -> jmp

			// change function that handles camera movement to not react to mouse movement
			// and not to fall back to WASD camera movement either (legacy code...?)
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x4464e5 : 0x446635);
				branch.WriteJump5Byte(0xe9);
				bin.WriteUInt8(0x90); // nop
				branch.SetTarget(jp ? 0x44667au : 0x4467cau);
			}

			// there's a third function at 0x53c766 that seems involved here, but leaving it alone seems to work just fine...

			// skip mouse movement processing function or something like that?
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x446e3a : 0x446f8a);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
		}

		public static void PatchShowMouseCursor(Stream bin, Sen1ExecutablePatchState state) {
			bool jp = state.IsJp;

			// remove call to ShowCursor(0)
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x7be0ea : 0x7bf9ba);
			for (int i = 0; i < 8; ++i) {
				bin.WriteUInt8(0x90); // nop
			}
		}

		public static void PatchDisablePauseOnFocusLoss(Stream bin, Sen1ExecutablePatchState state) {
			bool jp = state.IsJp;

			// 0x444AA0 -> game active setter
			// 0x444AF0 -> game active getter

			// don't silence audio output when unfocused
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x447d6a : 0x447eba);
				branch.WriteJump5Byte(0xe9);
				branch.SetTarget(jp ? 0x447da6u : 0x447ef6u);
			}

			// still run main game loop when unfocused
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x441c00 : 0x441d50);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop

			// avoid processing mouse clicks when unfocused
			// (this previously happened only implicitly because the game didn't run...)
			// carve out some now-unused code space
			uint codespaceStart = jp ? 0x447d6fu : 0x447ebfu;
			uint codespaceEnd = jp ? 0x447da4u : 0x447ef4u;
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

				back_to_function.SetTarget(jp ? 0x479ad8u : 0x47b348u);
				bin.Position = state.Mapper.MapRamToRom(jp ? 0x479ad4u : 0x47b344);
				ulong GetKeyStateAddress = bin.ReadUInt32(be);
				ulong GameStateAddress = (jp ? 0x1361c28u : 0x01363fc8u).ToEndian(be);

				bin.Position = state.Mapper.MapRamToRom(jp ? 0x479ad1 : 0x47b341);
				jump_to_codespace.WriteJump5Byte(0xe9);    // jmp jump_to_codespace
				bin.WriteUInt8(0x90);                      // nop
				bin.WriteUInt8(0x90);                      // nop

				bin.Position = state.Mapper.MapRamToRom(codespace.Address);
				jump_to_codespace.SetTarget(codespace.Address);
				bin.WriteUInt48(0x8b0d00000000u | GameStateAddress, be);   // mov ecx,[static_address_that_holds_game_state]
				bin.WriteUInt16(0x85c9, be);                               // test ecx,ecx
				skip_processing.WriteJump(0x74);                           // jz skip_processing
				bin.WriteUInt48(0x8a89b8070000, be);                       // mov cl,byte ptr[ecx+7b8h]   ; cl now holds 0 if unfocused, 1 if focused
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
