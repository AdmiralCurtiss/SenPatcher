using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
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
	public partial class Sen1Form : Form {
		private string Path;
		private Stream Binary;
		private Sen1ExecutablePatchInterface PatchInfo;

		public Sen1Form(string path, Stream binary, SenVersion version) {
			Path = path;
			Binary = binary;

			switch (version) {
				case SenVersion.Sen1_v1_6_En:
					PatchInfo = new Sen1ExecutablePatchEnglish();
					break;
				case SenVersion.Sen1_v1_6_Jp:
					PatchInfo = new Sen1ExecutablePatchJapanese();
					break;
				default:
					throw new Exception("Invalid version for Sen 1 patch form.");
			}

			InitializeComponent();
			labelFile.Text = path;
			labelVersion.Text = version == SenVersion.Sen1_v1_6_En ? "1.6 (English)" : "1.6 (Japanese)";

			comboBoxTurboModeKey.Items.Clear();
			comboBoxTurboModeKey.Items.Add("Square / X");
			comboBoxTurboModeKey.Items.Add("Cross / A");
			comboBoxTurboModeKey.Items.Add("Circle / B");
			comboBoxTurboModeKey.Items.Add("Triangle / Y");
			comboBoxTurboModeKey.Items.Add("L1 / LB");
			comboBoxTurboModeKey.Items.Add("R1 / RB");
			comboBoxTurboModeKey.Items.Add("L2 / LT");
			comboBoxTurboModeKey.Items.Add("R2 / RT");
			comboBoxTurboModeKey.Items.Add("Select / Back");
			comboBoxTurboModeKey.Items.Add("Start");
			comboBoxTurboModeKey.Items.Add("L3 / LS");
			comboBoxTurboModeKey.Items.Add("R3 / RS");
			comboBoxTurboModeKey.Items.Add("D-Pad Up");
			comboBoxTurboModeKey.Items.Add("D-Pad Right");
			comboBoxTurboModeKey.Items.Add("D-Pad Down");
			comboBoxTurboModeKey.Items.Add("D-Pad Left");
			comboBoxTurboModeKey.SelectedIndex = 7;
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
				bool allowR2NotebookShortcut = checkBoxAllowR2InTurboMode.Checked;
				int turboKey = comboBoxTurboModeKey.SelectedIndex;
				bool fixTextureIds = checkBoxFixHdTextureId.Checked;

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

				MessageBox.Show("File successfully patched.\n\nA backup has been created at " + backuppath + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				Close();
				return;
			}
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
