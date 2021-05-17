using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static partial class Sen1ExecutablePatches {
		public static void DisableMouseCapture(Stream bin, Sen1ExecutablePatchState state) {
			state.InitCodeSpaceIfNeeded(bin);

			// change function that captures the mouse cursor to not do that
			bin.Position = state.Mapper.MapRamToRom(0x5df536);
			bin.WriteByte(0xeb); // jz -> jmp

			// change function that handles camera movement to not react to mouse movement
			// and not to fall back to WASD camera movement either (legacy code...?)
			using (var branch = new BranchHelper4Byte(bin, state.Mapper)) {
				bin.Position = state.Mapper.MapRamToRom(0x446635);
				branch.WriteJump5Byte(0xe9);
				branch.SetTarget(0x4467ca);
			}

			// there's a third function at 0x53c766 that seems involved here, but leaving it alone seems to work just fine...

			// remove call to ShowCursor(0)
			bin.Position = state.Mapper.MapRamToRom(0x7bf9ba);
			bin.WriteUInt8(0xc3); // ret

			// seems not necessary with the changes above...
			// skip mouse movement processing function or something like that?
			//ms.Position = PatchInfo.Mapper.MapRamToRom(0x446f8a);
			//ms.WriteUInt8(0x90); // nop
			//ms.WriteUInt8(0x90); // nop
			//ms.WriteUInt8(0x90); // nop
			//ms.WriteUInt8(0x90); // nop
			//ms.WriteUInt8(0x90); // nop
		}

		public static void DisablePauseOnFocusLoss(Stream bin, Sen1ExecutablePatchState state) {
			// don't pause game on focus loss (don't treat WM_ACTIVATEAPP as special)
			bin.Position = state.Mapper.MapRamToRom(0x447a66);
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
			bin.WriteUInt8(0x90); // nop
		}
	}
}
