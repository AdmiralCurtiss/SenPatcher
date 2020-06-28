using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1 {
	public class Sen1PatchExec {
		public string Path { get; private set; }
		public string HumanReadableVersion { get; private set; }
		private Stream Binary;
		private Sen1ExecutablePatchInterface PatchInfo;
		private List<FileFix> AssetPatches;
		public int AssetPatchCount => AssetPatches.Count;

		public string BaseFolder => System.IO.Path.Combine(System.IO.Path.GetDirectoryName(Path), SenCommonPaths.Sen1BaseFromExe);

		public string BackupFolder => System.IO.Path.Combine(BaseFolder, SenCommonPaths.BackupFolder);

		public Sen1PatchExec(string path, Stream binary, SenVersion version) {
			Path = path;
			Binary = binary.CopyToMemory();

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

			AssetPatches = PatchInfo.GetFileFixes();
		}

		public PatchResult ApplyPatches(bool removeTurboSkip, bool allowR2NotebookShortcut, int turboKey, bool fixTextureIds, bool patchAssets) {
			int total = 0;
			int success = 0;

			using (MemoryStream ms = Binary.CopyToMemory()) {
				++total;
				SenUtils.CreateBackupIfRequired(System.IO.Path.Combine(BackupFolder, System.IO.Path.GetFileName(Path) + ".bin"), ms);

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
				if (SenUtils.TryWriteFile(ms, Path)) {
					++success;
				}
			}

			if (patchAssets) {
				foreach (var file in AssetPatches) {
					++total;
					if (file.TryApply(BaseFolder, BackupFolder)) {
						++success;
					}
				}
			}

			return new PatchResult(total, success);
		}

		public PatchResult RestoreOriginalFiles() {
			int total = 0;
			int success = 0;

			using (MemoryStream ms = Binary.CopyToMemory()) {
				++total;
				if (SenUtils.TryWriteFileIfDifferent(ms, Path)) {
					++success;
				}
			}

			foreach (var file in AssetPatches) {
				++total;
				if (file.TryRevert(BaseFolder, BackupFolder)) {
					++success;
				}
			}

			return new PatchResult(total, success);
		}
	}
}
