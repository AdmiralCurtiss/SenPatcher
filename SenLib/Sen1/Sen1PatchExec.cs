using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using System;
using System.IO;

namespace SenLib.Sen1 {
	public class Sen1PatchExec {
		public string Path { get; private set; }
		public string HumanReadableVersion { get; private set; }
		private Stream Binary;
		private Sen1ExecutablePatchInterface PatchInfo;

		public Sen1PatchExec(string path, Stream binary, SenVersion version) {
			Path = path;
			Binary = binary;

			switch (version) {
				case SenVersion.Sen1_v1_6_En:
					PatchInfo = new Sen1ExecutablePatchEnglish();
					HumanReadableVersion = "1.6 (English)";
					break;
				case SenVersion.Sen1_v1_6_Jp:
					PatchInfo = new Sen1ExecutablePatchJapanese();
					HumanReadableVersion = "1.6 (Japanese)";
					break;
				default:
					throw new Exception("Invalid version for Sen 1 patch form.");
			}
		}

		public bool ApplyPatches(bool removeTurboSkip, bool allowR2NotebookShortcut, int turboKey, bool fixTextureIds) {
			using (MemoryStream ms = Binary.CopyToMemory()) {
				// first create a backup
				string backuppath = Path + SenCommonPaths.BackupPostfix;
				using (var fs = new FileStream(backuppath, FileMode.Create, FileAccess.Write)) {
					ms.Position = 0;
					StreamUtils.CopyStream(ms, fs);
				}

				// patch data
				if (removeTurboSkip) {
					Sen1ExecutablePatches.PatchJumpBattleAnimationAutoSkip(ms, PatchInfo, true);
					Sen1ExecutablePatches.PatchJumpBattleResultsAutoSkip(ms, PatchInfo, true);
				}
				if (allowR2NotebookShortcut) {
					Sen1ExecutablePatches.PatchJumpR2NotebookOpen(ms, PatchInfo, true);
					Sen1ExecutablePatches.PatchJumpR2NotebookSettings(ms, PatchInfo, true);
				}
				if (turboKey >= 0 && turboKey <= 0xF) {
					Sen1ExecutablePatches.PatchButtonBattleAnimationAutoSkip(ms, PatchInfo, (byte)turboKey);
					Sen1ExecutablePatches.PatchButtonBattleResultsAutoSkip(ms, PatchInfo, (byte)turboKey);
					Sen1ExecutablePatches.PatchButtonTurboMode(ms, PatchInfo, (byte)turboKey);
				}
				if (fixTextureIds) {
					Sen1ExecutablePatches.PatchThorMasterQuartzString(ms, PatchInfo);
				}

				// write patched file
				using (var fs = new FileStream(Path, FileMode.Create, FileAccess.Write)) {
					ms.Position = 0;
					StreamUtils.CopyStream(ms, fs);
				}

				return true;
			}
		}

		public bool RestoreOriginalFiles() {
			using (MemoryStream ms = Binary.CopyToMemory()) {
				using (var fs = new FileStream(Path, FileMode.Create, FileAccess.Write)) {
					ms.Position = 0;
					StreamUtils.CopyStream(ms, fs);
				}

				return true;
			}
		}
	}
}
