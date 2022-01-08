using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen4 {
	public static partial class Sen4ExecutablePatches {
		public static void AllowSwitchToNightmare(Stream bin, Sen4ExecutablePatchState state) {
			bool jp = state.IsJp;
			EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;

			// allow nightmare to be selected when difficulty is currently not nightmare
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x140485d57 : 0x140488298);
			bin.WriteUInt32(5, le);

			// don't show warning when trying to switch away from nightmare
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x140210087 : 0x140212337);
			bin.WriteUInt32(0x224, le);
			bin.Position = state.Mapper.MapRamToRom(jp ? 0x1403d55b3 : 0x1403d7844);
			bin.WriteUInt32(0x224, le);
		}
	}
}
