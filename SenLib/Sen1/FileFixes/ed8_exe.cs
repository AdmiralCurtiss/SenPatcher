using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1.FileFixes {
	public class ed8_exe : FileMod {
		bool IsJp;
		bool RemoveTurboSkip;
		bool AllowR2NotebookShortcut;
		int TurboKey;
		bool FixTextureIds;
		bool CorrectLanguageVoiceTables;
		bool DisableMouseCapture;
		bool ShowMouseCursor;
		bool DisablePauseOnFocusLoss;
		bool FixArtsSupport;
		bool Force0Kerning;
		bool ForceXInput;

		public ed8_exe(
			bool jp,
			bool removeTurboSkip,
			bool allowR2NotebookShortcut,
			int turboKey,
			bool fixTextureIds,
			bool correctLanguageVoiceTables,
			bool disableMouseCapture,
			bool showMouseCursor,
			bool disablePauseOnFocusLoss,
			bool fixArtsSupport,
			bool force0Kerning,
			bool forceXInput
		) {
			IsJp = jp;
			RemoveTurboSkip = removeTurboSkip;
			AllowR2NotebookShortcut = allowR2NotebookShortcut;
			TurboKey = turboKey;
			FixTextureIds = fixTextureIds;
			CorrectLanguageVoiceTables = correctLanguageVoiceTables;
			DisableMouseCapture = disableMouseCapture;
			ShowMouseCursor = showMouseCursor;
			DisablePauseOnFocusLoss = disablePauseOnFocusLoss;
			FixArtsSupport = fixArtsSupport;
			Force0Kerning = force0Kerning;
			ForceXInput = forceXInput;
		}

		public string GetDescription() {
			return string.Format("Main executable code changes for {0} language.", IsJp ? "Japanese" : "English");
		}

		private HyoutaUtils.Checksum.SHA1 GetExecutableHash() {
			return IsJp ? new HyoutaUtils.Checksum.SHA1(0x1d56abf5aa02eeaeul, 0x334797c287ef2109ul, 0xc7a103fau) : new HyoutaUtils.Checksum.SHA1(0x373c1d1b30001af3ul, 0x60042365ed257e07ul, 0x0bf40accu);
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}

			var mapper = new PeExe(s, EndianUtils.Endianness.LittleEndian).CreateRomMapper();
			MemoryStream ms = s.CopyToMemoryAndDispose();

			Sen1ExecutablePatchState PatchInfo = new Sen1ExecutablePatchState(IsJp, mapper);
			PatchInfo.InitCodeSpaceIfNeeded(ms);

			if (RemoveTurboSkip) {
				Sen1ExecutablePatches.PatchJumpBattleAnimationAutoSkip(ms, PatchInfo);
				Sen1ExecutablePatches.PatchJumpBattleResultsAutoSkip(ms, PatchInfo);
			}
			if (AllowR2NotebookShortcut) {
				Sen1ExecutablePatches.PatchJumpR2NotebookOpen(ms, PatchInfo);
				Sen1ExecutablePatches.PatchJumpR2NotebookSettings(ms, PatchInfo);
			}
			if (TurboKey >= 0 && TurboKey <= 0xF) {
				Sen1ExecutablePatches.PatchButtonBattleAnimationAutoSkip(ms, PatchInfo, (byte)TurboKey);
				Sen1ExecutablePatches.PatchButtonBattleResultsAutoSkip(ms, PatchInfo, (byte)TurboKey);
				Sen1ExecutablePatches.PatchButtonTurboMode(ms, PatchInfo, (byte)TurboKey);
			}
			if (FixTextureIds) {
				Sen1ExecutablePatches.PatchThorMasterQuartzString(ms, PatchInfo);
			}
			if (CorrectLanguageVoiceTables) {
				Sen1ExecutablePatches.PatchLanguageAppropriateVoiceTables(ms, PatchInfo);
			}
			if (DisableMouseCapture) {
				Sen1ExecutablePatches.PatchDisableMouseCapture(ms, PatchInfo);
			}
			if (ShowMouseCursor) {
				Sen1ExecutablePatches.PatchShowMouseCursor(ms, PatchInfo);
			}
			if (DisablePauseOnFocusLoss) {
				Sen1ExecutablePatches.PatchDisablePauseOnFocusLoss(ms, PatchInfo);
			}
			if (FixArtsSupport) {
				Sen1ExecutablePatches.PatchFixArtsSupportCutin(ms, PatchInfo);
			}
			if (!IsJp && Force0Kerning) {
				Sen1ExecutablePatches.PatchForce0Kerning(ms, PatchInfo);
			}
			if (ForceXInput) {
				Sen1ExecutablePatches.PatchForceXInput(ms, PatchInfo);
			}

			if (IsJp) {
				// this inits the struct responsible for telling which button to use for the textbox text advance prompts
				// the JP version incorrectly uses the cancel button instead of the confirm button, so swap that around
				ms.Position = PatchInfo.Mapper.MapRamToRom(0x467222);
				ms.WriteUInt8(4);
			}

			// add indicator to the title screen that we're running a modified executable
			ms.Position = PatchInfo.Mapper.MapRamToRom(PatchInfo.PushAddressVersionString);
			uint addressVersionString = ms.ReadUInt32(EndianUtils.Endianness.LittleEndian);
			ms.Position = PatchInfo.Mapper.MapRamToRom(addressVersionString);
			string versionString = ms.ReadAsciiNullterm();
			string newVersionString = versionString + "  SenPatcher " + Version.SenPatcherVersion;
			MemoryStream newVersionStringStream = new MemoryStream();
			newVersionStringStream.WriteAsciiNullterm(newVersionString);
			byte[] newVersionStringBytes = newVersionStringStream.CopyToByteArrayAndDispose();
			var regionStrings = PatchInfo.RegionScriptCompilerFunctionStrings;
			uint addressNewVersionString = regionStrings.Address;
			ms.Position = PatchInfo.Mapper.MapRamToRom(regionStrings.Address);
			ms.Write(newVersionStringBytes);
			regionStrings.TakeToAddress(PatchInfo.Mapper.MapRomToRam(ms.Position), "SenPatcher version string");
			ms.Position = PatchInfo.Mapper.MapRamToRom(PatchInfo.PushAddressVersionString);
			ms.WriteUInt32(addressNewVersionString, EndianUtils.Endianness.LittleEndian);

			ms.Position = 0;
			return new FileModResult[] { new FileModResult(IsJp ? "ed8jp.exe" : "ed8.exe", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult(IsJp ? "ed8jp.exe" : "ed8.exe", s) };
		}
	}
}
