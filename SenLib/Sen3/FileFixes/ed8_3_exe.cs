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
		bool DisablePauseOnFocusLoss;
		bool FixControllerMapping;

		public ed8_3_exe(
			bool jp,
			bool fixInGameButtonMappingValidity,
			bool allowSwitchToNightmare,
			bool swapBrokenMasterQuartzValuesForDisplay,
			bool disableMouseCapture,
			bool disablePauseOnFocusLoss,
			bool fixControllerMapping
		) {
			IsJp = jp;
			FixInGameButtonMappingValidity = fixInGameButtonMappingValidity;
			AllowSwitchToNightmare = allowSwitchToNightmare;
			SwapBrokenMasterQuartzValuesForDisplay = swapBrokenMasterQuartzValuesForDisplay;
			DisableMouseCapture = disableMouseCapture;
			DisablePauseOnFocusLoss = disablePauseOnFocusLoss;
			FixControllerMapping = fixControllerMapping;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0x2e9da161efc30b9aul, 0xf020903e99cfa9a2ul, 0x287ff1a2u) : new HyoutaUtils.Checksum.SHA1(0x72920fa16072f4b7ul, 0x51e6a5be3fb4df50ul, 0xb21da506u);
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
			if (DisablePauseOnFocusLoss) {
				Sen3ExecutablePatches.PatchDisablePauseOnFocusLoss(ms, PatchInfo);
			}

			// add indicator to the title screen that we're running a modified executable
			{
				var state = PatchInfo;
				long loadAddrTitleScreenVersionString = (IsJp ? 0x1404219f6 : 0x14042cff9) + 3;
				long loadAddrCrashRptVersionString = (IsJp ? 0x14012c3a9 : 0x14012f769) + 3;

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
				ms.Position = state.Mapper.MapRamToRom(loadAddrCrashRptVersionString);
				ms.WriteUInt32((uint)(addressNewVersionString - loadAddrCrashRptVersionStringOffset) + (uint)(versionString.Length - 4), EndianUtils.Endianness.LittleEndian);
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
