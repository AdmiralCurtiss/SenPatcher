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

		public static void PatchButtonTurboMode(Stream binary, Sen1ExecutablePatchState patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressButtonTurboMode);
			binary.WriteUInt8(button);
		}

		public static void PatchButtonBattleAnimationAutoSkip(Stream binary, Sen1ExecutablePatchState patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressButtonBattleAnimationAutoSkip);
			binary.WriteUInt8(button);
		}

		public static void PatchButtonBattleResultsAutoSkip(Stream binary, Sen1ExecutablePatchState patchInfo, byte button) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressButtonBattleResultsAutoSkip);
			binary.WriteUInt8(button);
		}

		public static void PatchJumpBattleAnimationAutoSkip(Stream binary, Sen1ExecutablePatchState patchInfo) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressJumpBattleAnimationAutoSkip);
			binary.WriteUInt16(0xeb07, EndianUtils.Endianness.BigEndian); // jmp
		}

		public static void PatchJumpBattleResultsAutoSkip(Stream binary, Sen1ExecutablePatchState patchInfo) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressJumpBattleResultsAutoSkip);
			binary.WriteUInt40(0xe981010000, EndianUtils.Endianness.BigEndian); // jmp
			binary.WriteUInt8(0x90); // nop
		}

		public static void PatchJumpR2NotebookOpen(Stream binary, Sen1ExecutablePatchState patchInfo) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressJumpR2NotebookOpen);
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
		}

		public static void PatchJumpR2NotebookSettings(Stream binary, Sen1ExecutablePatchState patchInfo) {
			binary.Position = (long)new Sen1Mapper().MapRamToRom((ulong)patchInfo.AddressJumpR2NotebookSettings);
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
			binary.WriteUInt8(0x90); // nop
		}

		public static void PatchThorMasterQuartzString(Stream binary, Sen1ExecutablePatchState patchInfo) {
			binary.Position = patchInfo.RomAddressThorMasterQuartzTextureIdTypo;
			long p = binary.Position;
			byte[] tmp = binary.ReadUInt8Array(4);
			binary.Position = p;
			binary.Write(tmp, 1, 3);
		}
	}
}
