using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		// TODO:
		// the branch at ram 0x57C807 or 0x57C80D seems responsible for the weird behavior 
		// where the same music track doesn't restart if it is still fading out (and the wrong one gets queued instead)
		// this probably has side-effects if we just always jump but investigate this further...

		public static void PatchJumpBattleAnimationAutoSkip(Stream binary, Sen2ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen2Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleAnimationAutoSkip());
			if (enable) {
				binary.WriteUInt16(0xeb07, EndianUtils.Endianness.BigEndian); // jmp
			} else {
				binary.WriteUInt16(0x7407, EndianUtils.Endianness.BigEndian); // jz
			}
		}

		public static void PatchJumpBattleStartAutoSkip(Stream binary, Sen2ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen2Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleStartAutoSkip());
			if (enable) {
				binary.WriteUInt16(0xeb24, EndianUtils.Endianness.BigEndian); // jmp
			} else {
				binary.WriteUInt16(0x7424, EndianUtils.Endianness.BigEndian); // jz
			}
		}

		public static void PatchJumpBattleSomethingAutoSkip(Stream binary, Sen2ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen2Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleSomethingAutoSkip());
			if (enable) {
				binary.WriteUInt16(0xeb0d, EndianUtils.Endianness.BigEndian); // jmp
			} else {
				binary.WriteUInt16(0x740d, EndianUtils.Endianness.BigEndian); // jz
			}
		}

		public static void PatchJumpBattleResultsAutoSkip(Stream binary, Sen2ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen2Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleResultsAutoSkip());
			if (enable) {
				binary.WriteUInt40(0xe96b010000, EndianUtils.Endianness.BigEndian); // jmp
				binary.WriteUInt8(0x90); // nop
			} else {
				binary.WriteUInt48(0x0f846a010000, EndianUtils.Endianness.BigEndian); // jz
			}
		}
	}
}
