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
		private Sen1PatchExec Exec;

		public Sen1Form(string path, Stream binary, SenVersion version) {
			Exec = new Sen1PatchExec(path, binary, version);

			InitializeComponent();
			labelFile.Text = Exec.Path;
			labelVersion.Text = Exec.HumanReadableVersion;

			int assetPatchCount = Exec.AssetPatchCount;
			if (assetPatchCount == 0) {
				checkBoxAssetPatches.Text = "No known script/asset fixes for this language";
				checkBoxAssetPatches.Checked = false;
				checkBoxAssetPatches.Enabled = false;
			} else {
				checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";
			}

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
			bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
			bool allowR2NotebookShortcut = checkBoxAllowR2InTurboMode.Checked;
			int turboKey = comboBoxTurboModeKey.SelectedIndex;
			bool fixTextureIds = checkBoxFixHdTextureId.Checked;
			bool patchAssets = checkBoxAssetPatches.Checked;

			PatchResult result;
			try {
				result = Exec.ApplyPatches(removeTurboSkip, allowR2NotebookShortcut, turboKey, fixTextureIds, patchAssets);
			} catch (Exception ex) {
				MessageBox.Show("Exception occurred: " + ex.ToString());
				return;
			}

			if (result.AllSuccessful) {
				MessageBox.Show("Patch successful.\n\nA backup has been created at " + Path.GetFullPath(Exec.BackupFolder) + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				Close();
				return;
			} else {
				MessageBox.Show(
					  "Patching failed for " + result.FailedFiles + " file" + (result.FailedFiles == 1 ? "" : "s") + ".\n"
					+ "Verify that the game files are writable and not corrupted."
				);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			PatchResult result;
			try {
				result = Exec.RestoreOriginalFiles();
			} catch (Exception ex) {
				MessageBox.Show("Exception occurred: " + ex.ToString());
				return;
			}

			if (result.AllSuccessful) {
				MessageBox.Show("Original " + (result.SuccessfulFiles == 1 ? "file has" : "files have") + " been restored.");
				return;
			} else {
				MessageBox.Show(
					  "Restoration failed for " + result.FailedFiles + " file" + (result.FailedFiles == 1 ? "" : "s") + ".\n"
					+ "Verify that the game files are writable and not corrupted."
				);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 1", Exec.AssetPatchDescriptions).ShowDialog();
		}
	}
}
