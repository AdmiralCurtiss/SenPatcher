using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen4.FileFixes {
	public class ed8_4_exe : FileMod {
		bool IsJp;
		bool AllowSwitchToNightmare;
		bool DisableMouseCapture;
		bool DisablePauseOnFocusLoss;
		bool SeparateSwapConfirmCancelOption;
		bool FixSwappedButtonsWhenDynamicPromptsOff;

		public ed8_4_exe(
			bool jp,
			bool allowSwitchToNightmare,
			bool disableMouseCapture,
			bool disablePauseOnFocusLoss,
			bool separateSwapConfirmCancelOption,
			bool fixSwappedButtonsWhenDynamicPromptsOff
		) {
			IsJp = jp;
			AllowSwitchToNightmare = allowSwitchToNightmare;
			DisableMouseCapture = disableMouseCapture;
			DisablePauseOnFocusLoss = disablePauseOnFocusLoss;
			SeparateSwapConfirmCancelOption = separateSwapConfirmCancelOption;
			FixSwappedButtonsWhenDynamicPromptsOff = fixSwappedButtonsWhenDynamicPromptsOff;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0xf3952664b8da2607ul, 0x23677c91980ef9bcul, 0xf5c3113au) : new HyoutaUtils.Checksum.SHA1(0xc8669cb40e015c16ul, 0xec16776bd9a30945ul, 0xb5545a72u);
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}

			var mapper = new PeExe(s, EndianUtils.Endianness.LittleEndian).CreateRomMapper();
			MemoryStream ms = s.CopyToMemoryAndDispose();

			Sen4ExecutablePatchState PatchInfo = new Sen4ExecutablePatchState(IsJp, mapper, ms);

			if (AllowSwitchToNightmare) {
				Sen4ExecutablePatches.AllowSwitchToNightmare(ms, PatchInfo);
			}
			if (DisableMouseCapture) {
				Sen4ExecutablePatches.PatchDisableMouseCapture(ms, PatchInfo);
			}
			if (DisablePauseOnFocusLoss) {
				Sen4ExecutablePatches.PatchDisablePauseOnFocusLoss(ms, PatchInfo);
			}

			// add indicator to the title screen that we're running a modified executable
			{
				var state = PatchInfo;
				long loadAddrTitleScreenVersionString = (IsJp ? 0x14047b07a : 0x14047d1bd) + 3;
				long loadAddrCrashRptVersionString = (IsJp ? 0x140580a26 : 0x140582fb6) + 3;

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
				var regionStrings = state.RegionScriptCompilerFunctionStrings;
				long addressNewVersionString = regionStrings.Address;
				ms.Position = state.Mapper.MapRamToRom(regionStrings.Address);
				ms.Write(newVersionStringBytes);
				regionStrings.TakeToAddress(state.Mapper.MapRomToRam(ms.Position), "SenPatcher version string");
				ms.Position = state.Mapper.MapRamToRom(loadAddrTitleScreenVersionString);
				ms.WriteUInt32((uint)(addressNewVersionString - loadAddrTitleScreenVersionStringOffset), EndianUtils.Endianness.LittleEndian);
				ms.Position = state.Mapper.MapRamToRom(loadAddrCrashRptVersionString);
				ms.WriteUInt32((uint)(addressNewVersionString - loadAddrCrashRptVersionStringOffset) + (uint)(versionString.Length - 3), EndianUtils.Endianness.LittleEndian);
			}

			if (SeparateSwapConfirmCancelOption) {
				Sen4ExecutablePatches.PatchSeparateSwapConfirmCancelOption(ms, PatchInfo);
			}
			if (FixSwappedButtonsWhenDynamicPromptsOff) {
				Sen4ExecutablePatches.PatchFixPcConfirmCancelWhenSwapped(ms, PatchInfo);
			}

			return new FileModResult[] { new FileModResult(IsJp ? "bin/Win64/ed8_4_PC_JP.exe" : "bin/Win64/ed8_4_PC.exe", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult(IsJp ? "bin/Win64/ed8_4_PC_JP.exe" : "bin/Win64/ed8_4_PC.exe", s) };
		}
	}
}
