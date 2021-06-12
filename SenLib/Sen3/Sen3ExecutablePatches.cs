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
	}
}
