using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public static class Sen1ExecutablePatches {
		// button IDs:
		// 0x0 = Square
		// 0x1 = Cross
		// 0x2 = Circle
		// 0x3 = Triangle
		// 0x4 = L1
		// 0x5 = R1
		// 0x6 = L2
		// 0x7 = R2
		// 0x8 = Select
		// 0x9 = Start
		// 0xA = L3
		// 0xB = R3
		// 0xC = D-Pad Up
		// 0xD = D-Pad Right
		// 0xE = D-Pad Down
		// 0xF = D-Pad Left

		public static void PatchButtonTurboMode(Stream binary, Sen1ExecutablePatchInterface patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressButtonTurboMode());
			binary.WriteUInt8(button);
		}

		public static void PatchButtonBattleAnimationAutoSkip(Stream binary, Sen1ExecutablePatchInterface patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressButtonBattleAnimationAutoSkip());
			binary.WriteUInt8(button);
		}

		public static void PatchButtonBattleResultsAutoSkip(Stream binary, Sen1ExecutablePatchInterface patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressButtonBattleResultsAutoSkip());
			binary.WriteUInt8(button);
		}

		public static void PatchJumpBattleAnimationAutoSkip(Stream binary, Sen1ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleAnimationAutoSkip());
			if (enable) {
				binary.WriteUInt16(0xeb07, EndianUtils.Endianness.BigEndian); // jmp
			} else {
				binary.WriteUInt16(0x7407, EndianUtils.Endianness.BigEndian); // jz
			}
		}

		public static void PatchJumpBattleResultsAutoSkip(Stream binary, Sen1ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpBattleResultsAutoSkip());
			if (enable) {
				binary.WriteUInt40(0xe981010000, EndianUtils.Endianness.BigEndian); // jmp
				binary.WriteUInt8(0x90); // nop
			} else {
				binary.WriteUInt48(0x0f8480010000, EndianUtils.Endianness.BigEndian); // jz
			}
		}

		public static void PatchJumpR2NotebookOpen(Stream binary, Sen1ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpR2NotebookOpen());
			if (enable) {
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
			} else {
				binary.WriteUInt16(0x7528, EndianUtils.Endianness.BigEndian); // jnz
			}
		}

		public static void PatchJumpR2NotebookSettings(Stream binary, Sen1ExecutablePatchInterface patchInfo, bool enable) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.GetAddressJumpR2NotebookSettings());
			if (enable) {
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
				binary.WriteUInt8(0x90); // nop
			} else {
				binary.WriteUInt48(0x0f85d9000000, EndianUtils.Endianness.BigEndian); // jnz
			}
		}

		public static void PatchThorMasterQuartzString(Stream binary, Sen1ExecutablePatchInterface patchInfo) {
			binary.Position = patchInfo.GetRomAddressThorMasterQuartzTextureIdTypo();
			long p = binary.Position;
			byte[] tmp = binary.ReadUInt8Array(4);
			binary.Position = p;
			binary.Write(tmp, 1, 3);
		}
	}
}
