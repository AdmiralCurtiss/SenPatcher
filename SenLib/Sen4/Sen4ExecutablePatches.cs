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

			// 1.2.0
			//long setMaxDifficultyIndexPos = (jp ? 0x140485d57 : 0x140488297) + 1;
			//long switchFromNightmareWarning1 = (jp ? 0x140210086 : 0x140212336) + 1;
			//long switchFromNightmareWarning2 = (jp ? 0x1403d55b2 : 0x1403d7842) + 1;

			// 1.2.1
			long setMaxDifficultyIndexPos = (jp ? 0x140486117 : 0x140488647) + 1;
			long switchFromNightmareWarning1 = (jp ? 0x140210076 : 0x140212326) + 1;
			long switchFromNightmareWarning2 = (jp ? 0x1403d5842 : 0x1403d7ac2) + 1;

			// allow nightmare to be selected when difficulty is currently not nightmare
			bin.Position = state.Mapper.MapRamToRom(setMaxDifficultyIndexPos);
			bin.WriteUInt32(5, le);

			// don't show warning when trying to switch away from nightmare
			bin.Position = state.Mapper.MapRamToRom(switchFromNightmareWarning1);
			bin.WriteUInt32(0x224, le);
			bin.Position = state.Mapper.MapRamToRom(switchFromNightmareWarning2);
			bin.WriteUInt32(0x224, le);
		}
	}
}
