using HyoutaUtils;
using SenLib;
using SenLib.Sen2;
using System;
using System.IO;

namespace SenLib.Sen2 {
	public class Sen2PatchExec {
		public string Path { get; private set; }
		public string HumanReadableVersion { get; private set; }
		private Stream Binary;
		private Sen2ExecutablePatchInterface PatchInfo;

		public string BackupFolder => System.IO.Path.Combine(System.IO.Path.GetDirectoryName(Path), SenCommonPaths.Sen2BaseFromExe, SenCommonPaths.BackupFolder);

		public Sen2PatchExec(string path, Stream binary, SenVersion version) {
			Path = path;
			Binary = binary;

			switch (version) {
				case SenVersion.Sen2_v1_4_1_En:
					PatchInfo = new Sen2ExecutablePatchEnglish();
					HumanReadableVersion = "1.4.1 (English)";
					break;
				case SenVersion.Sen2_v1_4_2_En:
					PatchInfo = new Sen2ExecutablePatchEnglish();
					HumanReadableVersion = "1.4.2 (English)";
					break;
				case SenVersion.Sen2_v1_4_1_Jp:
					PatchInfo = new Sen2ExecutablePatchJapanese();
					HumanReadableVersion = "1.4.1 (Japanese)";
					break;
				case SenVersion.Sen2_v1_4_2_Jp:
					PatchInfo = new Sen2ExecutablePatchJapanese();
					HumanReadableVersion = "1.4.2 (Japanese)";
					break;
				default:
					throw new Exception("Invalid version for Sen 2 patch form.");
			}
		}

		public bool ApplyPatches(bool removeTurboSkip, bool patchAudioThread, int audioThreadDivisor, bool patchBgmQueueing) {
			using (MemoryStream ms = Binary.CopyToMemory()) {
				SenUtils.CreateBackupIfRequired(System.IO.Path.Combine(BackupFolder, System.IO.Path.GetFileName(Path) + ".bin"), ms);

				// patch data
				if (removeTurboSkip) {
					Sen2ExecutablePatches.PatchJumpBattleAnimationAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleStartAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleSomethingAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleResultsAutoSkip(ms, PatchInfo, true);
				}

				if (patchAudioThread || patchBgmQueueing) {
					var state = new Sen2ExecutablePatchState();
					if (patchAudioThread) {
						Sen2ExecutablePatches.PatchMusicFadeTiming(ms, PatchInfo, state, audioThreadDivisor <= 0 ? 1000 : (uint)audioThreadDivisor);
					}
					if (patchBgmQueueing) {
						Sen2ExecutablePatches.PatchMusicQueueingOnSoundThreadSide(ms, PatchInfo, state);
					}
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
