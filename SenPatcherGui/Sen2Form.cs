using HyoutaUtils;
using SenLib;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class Sen2Form : Form {
		private string Path;
		private Stream Binary;
		private Sen2ExecutablePatchInterface PatchInfo;

		public Sen2Form(string path, Stream binary, SenVersion version) {
			Path = path;
			Binary = binary;

			InitializeComponent();
			labelFile.Text = path;

			switch (version) {
				case SenVersion.Sen2_v1_4_1_En:
					PatchInfo = new Sen2ExecutablePatchEnglish();
					labelVersion.Text = "1.4.1 (English)";
					break;
				case SenVersion.Sen2_v1_4_2_En:
					PatchInfo = new Sen2ExecutablePatchEnglish();
					labelVersion.Text = "1.4.2 (English)";
					break;
				case SenVersion.Sen2_v1_4_1_Jp:
					PatchInfo = new Sen2ExecutablePatchJapanese();
					labelVersion.Text = "1.4.1 (Japanese)";
					break;
				case SenVersion.Sen2_v1_4_2_Jp:
					PatchInfo = new Sen2ExecutablePatchJapanese();
					labelVersion.Text = "1.4.2 (Japanese)";
					break;
				default:
					throw new Exception("Invalid version for Sen 2 patch form.");
			}
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			using (MemoryStream ms = Binary.CopyToMemory()) {
				// first create a backup
				string backuppath = Path + SenCommonPaths.BackupPostfix;
				using (var fs = new FileStream(backuppath, FileMode.Create, FileAccess.Write)) {
					ms.Position = 0;
					StreamUtils.CopyStream(ms, fs);
				}

				// patch data
				bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
				bool patchAudioThread = checkBoxPatchAudioThread.Checked;
				bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;

				if (removeTurboSkip) {
					Sen2ExecutablePatches.PatchJumpBattleAnimationAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleStartAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleSomethingAutoSkip(ms, PatchInfo, true);
					Sen2ExecutablePatches.PatchJumpBattleResultsAutoSkip(ms, PatchInfo, true);
				}

				if (patchAudioThread || patchBgmQueueing) {
					var state = new Sen2ExecutablePatchState();
					if (patchAudioThread) {
						int divisor = (int)numericUpDownTicksPerSecond.Value;
						Sen2ExecutablePatches.PatchMusicFadeTiming(ms, PatchInfo, state, divisor <= 0 ? 1350 : (uint)divisor);
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

				MessageBox.Show("File successfully patched.\n\nA backup has been created at " + backuppath + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				Close();
				return;
			}
		}

		private void checkBoxPatchAudioThread_CheckedChanged(object sender, EventArgs e) {
			numericUpDownTicksPerSecond.Enabled = checkBoxPatchAudioThread.Checked;
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			using (MemoryStream ms = Binary.CopyToMemory()) {
				using (var fs = new FileStream(Path, FileMode.Create, FileAccess.Write)) {
					ms.Position = 0;
					StreamUtils.CopyStream(ms, fs);
				}
				MessageBox.Show("Original executable has been restored.");
				return;
			}
		}
	}
}
