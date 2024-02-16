using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3.FileFixes {
	public class ed8_3_exe : FileMod {
		bool IsJp;
		bool FixInGameButtonMappingValidity;
		bool AllowSwitchToNightmare;
		bool SwapBrokenMasterQuartzValuesForDisplay;
		bool DisableMouseCapture;
		bool ShowMouseCursor;
		bool DisablePauseOnFocusLoss;
		bool FixControllerMapping;
		bool ForceXInput;

		public ed8_3_exe(
			bool jp,
			bool fixInGameButtonMappingValidity,
			bool allowSwitchToNightmare,
			bool swapBrokenMasterQuartzValuesForDisplay,
			bool disableMouseCapture,
			bool showMouseCursor,
			bool disablePauseOnFocusLoss,
			bool fixControllerMapping,
			bool forceXInput
		) {
			IsJp = jp;
			FixInGameButtonMappingValidity = fixInGameButtonMappingValidity;
			AllowSwitchToNightmare = allowSwitchToNightmare;
			SwapBrokenMasterQuartzValuesForDisplay = swapBrokenMasterQuartzValuesForDisplay;
			DisableMouseCapture = disableMouseCapture;
			ShowMouseCursor = showMouseCursor;
			DisablePauseOnFocusLoss = disablePauseOnFocusLoss;
			FixControllerMapping = fixControllerMapping;
			ForceXInput = forceXInput;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0x14af222cbeee7c43ul, 0x4169c6a5bf2d21f6ul, 0x88000a26u) : new HyoutaUtils.Checksum.SHA1(0xa48022b2504c430ful, 0x2f017ab580aa6516ul, 0x638ed104u);
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}

			var mapper = new PeExe(s, EndianUtils.Endianness.LittleEndian).CreateRomMapper();
			MemoryStream ms = s.CopyToMemoryAndDispose();

			Sen3ExecutablePatchState PatchInfo = new Sen3ExecutablePatchState(IsJp, mapper, ms);

			if (FixInGameButtonMappingValidity) {
				Sen3ExecutablePatches.FixInGameButtonMappingValidity(ms, PatchInfo);
			}
			if (AllowSwitchToNightmare) {
				Sen3ExecutablePatches.AllowSwitchToNightmare(ms, PatchInfo);
			}
			if (!IsJp && SwapBrokenMasterQuartzValuesForDisplay) { // JP doesn't have this bug, so nothing to fix
				Sen3ExecutablePatches.SwapBrokenMasterQuartzValuesForDisplay(ms, PatchInfo);
			}
			if (DisableMouseCapture) {
				Sen3ExecutablePatches.PatchDisableMouseCapture(ms, PatchInfo);
			}
			if (ShowMouseCursor) {
				Sen3ExecutablePatches.PatchShowMouseCursor(ms, PatchInfo);
			}
			if (DisablePauseOnFocusLoss) {
				Sen3ExecutablePatches.PatchDisablePauseOnFocusLoss(ms, PatchInfo);
			}
			if (ForceXInput) {
				Sen3ExecutablePatches.PatchForceXInput(ms, PatchInfo);
			}

			// call CreateMutex without names so multiple instances of CS don't block eachother
			// 1.05
			//long createMutexString1 = IsJp ? 0x14054f839 : 0x14055b9d9;
			//long createMutexString2 = IsJp ? 0x14011c5e1 : 0x14011cae1;

			// 1.06
			long createMutexString1 = IsJp ? 0x14054fb09 : 0x14055bc99;
			long createMutexString2 = IsJp ? 0x14011c611 : 0x14011cb11;

			ms.Position = PatchInfo.Mapper.MapRamToRom(createMutexString1);
			ms.WriteUInt24(0x4d31c0, EndianUtils.Endianness.BigEndian); // xor r8,r8
			ms.Position = PatchInfo.Mapper.MapRamToRom(createMutexString2);
			ms.WriteUInt56(0x49c7c000000000, EndianUtils.Endianness.BigEndian); // mov r8,0

			// add indicator to the title screen that we're running a modified executable
			{
				// 1.05
				//long loadAddrTitleScreenVersionString = (IsJp ? 0x1404219f6 : 0x14042cff9) + 3;
				//long loadAddrCrashRptVersionString = (IsJp ? 0x14012c3a9 : 0x14012f769) + 3;

				// 1.06
				long loadAddrTitleScreenVersionString = (IsJp ? 0x140421cc6 : 0x14042d2b9) + 3;
				long loadAddrCrashRptVersionString = (IsJp ? 0x14012c3d9 : 0x14012f799) + 3;

				var state = PatchInfo;
				long loadAddrTitleScreenVersionStringOffset = loadAddrTitleScreenVersionString + 4;
				long loadAddrCrashRptVersionStringOffset = loadAddrCrashRptVersionString + 4;
				ms.Position = state.Mapper.MapRamToRom(loadAddrTitleScreenVersionString);
				long relativeAddressVersionString = ms.ReadUInt32(EndianUtils.Endianness.LittleEndian);
				long addressVersionString = loadAddrTitleScreenVersionStringOffset + relativeAddressVersionString;
				ms.Position = state.Mapper.MapRamToRom(addressVersionString);
				string versionString = ms.ReadAsciiNullterm();
				string newVersionString = versionString + "  SenPatcher " + Version.SenPatcherVersion;
				MemoryStream newVersionStringStream = new MemoryStream();
				newVersionStringStream.WriteAsciiNullterm(newVersionString);
				byte[] newVersionStringBytes = newVersionStringStream.CopyToByteArrayAndDispose();
				var regionStrings = IsJp ? state.RegionScriptCompilerFunctionStrings1 : state.RegionScriptCompilerFunctionStrings2;
				long addressNewVersionString = regionStrings.Address;
				ms.Position = state.Mapper.MapRamToRom(regionStrings.Address);
				ms.Write(newVersionStringBytes);
				regionStrings.TakeToAddress(state.Mapper.MapRomToRam(ms.Position), "SenPatcher version string");
				ms.Position = state.Mapper.MapRamToRom(loadAddrTitleScreenVersionString);
				ms.WriteUInt32((uint)(addressNewVersionString - loadAddrTitleScreenVersionStringOffset), EndianUtils.Endianness.LittleEndian);

				// this is using mbstowcs with a fixed length into a stack buffer, this needs more adjustments to be safe
				//ms.Position = state.Mapper.MapRamToRom(loadAddrCrashRptVersionString);
				//ms.WriteUInt32((uint)(addressNewVersionString - loadAddrCrashRptVersionStringOffset) + (uint)(versionString.Length - 4), EndianUtils.Endianness.LittleEndian);
			}

			if (FixControllerMapping) {
				Sen3ExecutablePatches.PatchFixControllerMappings(ms, PatchInfo);
			}

			return new FileModResult[] { new FileModResult(IsJp ? "bin/x64/ed8_3_PC_JP.exe" : "bin/x64/ed8_3_PC.exe", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult(IsJp ? "bin/x64/ed8_3_PC_JP.exe" : "bin/x64/ed8_3_PC.exe", s) };
		}
	}
}
