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

		public ed8_exe(bool jp, bool removeTurboSkip, bool allowR2NotebookShortcut, int turboKey, bool fixTextureIds) {
			IsJp = jp;
			RemoveTurboSkip = removeTurboSkip;
			AllowR2NotebookShortcut = allowR2NotebookShortcut;
			TurboKey = turboKey;
			FixTextureIds = fixTextureIds;
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

			MemoryStream ms = s.CopyToMemoryAndDispose();

			Sen1ExecutablePatchInterface PatchInfo = IsJp ? new Sen1ExecutablePatchJapanese() : (Sen1ExecutablePatchInterface)new Sen1ExecutablePatchEnglish();

			if (RemoveTurboSkip) {
				Sen1ExecutablePatches.PatchJumpBattleAnimationAutoSkip(ms, PatchInfo, true);
				Sen1ExecutablePatches.PatchJumpBattleResultsAutoSkip(ms, PatchInfo, true);
			}
			if (AllowR2NotebookShortcut) {
				Sen1ExecutablePatches.PatchJumpR2NotebookOpen(ms, PatchInfo, true);
				Sen1ExecutablePatches.PatchJumpR2NotebookSettings(ms, PatchInfo, true);
			}
			if (TurboKey >= 0 && TurboKey <= 0xF) {
				Sen1ExecutablePatches.PatchButtonBattleAnimationAutoSkip(ms, PatchInfo, (byte)TurboKey);
				Sen1ExecutablePatches.PatchButtonBattleResultsAutoSkip(ms, PatchInfo, (byte)TurboKey);
				Sen1ExecutablePatches.PatchButtonTurboMode(ms, PatchInfo, (byte)TurboKey);
			}
			if (FixTextureIds) {
				Sen1ExecutablePatches.PatchThorMasterQuartzString(ms, PatchInfo);
			}

			ms.Position = 0;
			return new FileModResult[] { new FileModResult(IsJp ? "ed8.exe" : "ed8jp.exe", ms) };
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			var s = storage.TryGetDuplicate(GetExecutableHash());
			if (s == null) {
				return null;
			}
			return new FileModResult[] { new FileModResult(IsJp ? "ed8.exe" : "ed8jp.exe", s) };
		}
	}
}
