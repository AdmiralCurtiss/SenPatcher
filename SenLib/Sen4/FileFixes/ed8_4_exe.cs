using HyoutaUtils;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen4.FileFixes {
	public class ed8_4_exe : FileMod {
		bool IsJp;
		bool AllowSwitchToNightmare;
		bool DisableMouseCapture;
		bool ShowMouseCursor;
		bool DisablePauseOnFocusLoss;
		bool SeparateSwapConfirmCancelOption;
		bool DefaultSwapConfirmCancelOptionOn;
		bool FixSwappedButtonsWhenDynamicPromptsOff;

		public ed8_4_exe(
			bool jp,
			bool allowSwitchToNightmare,
			bool disableMouseCapture,
			bool showMouseCursor,
			bool disablePauseOnFocusLoss,
			bool separateSwapConfirmCancelOption,
			bool defaultSwapConfirmCancelOptionOn,
			bool fixSwappedButtonsWhenDynamicPromptsOff
		) {
			IsJp = jp;
			AllowSwitchToNightmare = allowSwitchToNightmare;
			DisableMouseCapture = disableMouseCapture;
			ShowMouseCursor = showMouseCursor;
			DisablePauseOnFocusLoss = disablePauseOnFocusLoss;
			SeparateSwapConfirmCancelOption = separateSwapConfirmCancelOption;
			DefaultSwapConfirmCancelOptionOn = defaultSwapConfirmCancelOptionOn;
			FixSwappedButtonsWhenDynamicPromptsOff = fixSwappedButtonsWhenDynamicPromptsOff;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0x79e81f7a977e9180ul, 0x41b66fb27dc8805dul, 0x035e83ffu) : new HyoutaUtils.Checksum.SHA1(0x0d424434330d4368ul, 0xc1ae187cae83e4dbul, 0xa296ccf1u);
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
			if (ShowMouseCursor) {
				Sen4ExecutablePatches.PatchShowMouseCursor(ms, PatchInfo);
			}
			if (DisablePauseOnFocusLoss) {
				Sen4ExecutablePatches.PatchDisablePauseOnFocusLoss(ms, PatchInfo);
			}

			// call CreateMutex without names so multiple instances of CS don't block eachother
			// 1.2.0
			//long createMutexString1 = IsJp ? 0x1405adea9 : 0x1405b0439;
			//long createMutexString2 = IsJp ? 0x1400d6041 : 0x1400d60c1;

			// 1.2.1
			long createMutexString1 = IsJp ? 0x1405ae269 : 0x1405b07e9;
			long createMutexString2 = IsJp ? 0x1400d6051 : 0x1400d60d1;

			ms.Position = PatchInfo.Mapper.MapRamToRom(createMutexString1);
			ms.WriteUInt24(0x4d31c0, EndianUtils.Endianness.BigEndian); // xor r8,r8
			ms.Position = PatchInfo.Mapper.MapRamToRom(createMutexString2);
			ms.WriteUInt56(0x49c7c000000000, EndianUtils.Endianness.BigEndian); // mov r8,0

			// add indicator to the title screen that we're running a modified executable
			{
				// 1.2.0
				//long loadAddrTitleScreenVersionString = (IsJp ? 0x14047b07a : 0x14047d1bd) + 3;
				//long loadAddrCrashRptVersionString = (IsJp ? 0x140580a26 : 0x140582fb6) + 3;

				// 1.2.1
				long loadAddrTitleScreenVersionString = (IsJp ? 0x14047b43a : 0x14047d56d) + 3;
				long loadAddrCrashRptVersionString = (IsJp ? 0x140580de6 : 0x140583366) + 3;

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
				Sen4ExecutablePatches.PatchSeparateSwapConfirmCancelOption(ms, PatchInfo, DefaultSwapConfirmCancelOptionOn);
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
