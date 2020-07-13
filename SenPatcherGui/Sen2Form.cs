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
		private Sen2PatchExec Exec;

		public Sen2Form(string path, Stream binary, SenVersion version) {
			Exec = new Sen2PatchExec(path, binary, version);

			InitializeComponent();
			labelFile.Text = path;
			labelVersion.Text = Exec.HumanReadableVersion;

			int assetPatchCount = Exec.AssetPatchCount;
			if (assetPatchCount == 0) {
				checkBoxAssetPatches.Text = "No known script/asset fixes for this language";
				checkBoxAssetPatches.Checked = false;
				checkBoxAssetPatches.Enabled = false;
				buttonAssetFixDetails.Enabled = false;
			} else {
				checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";
			}
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
			bool patchAudioThread = checkBoxPatchAudioThread.Checked;
			int audioThreadDivisor = 1000;
			bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;
			bool patchAssets = checkBoxAssetPatches.Checked;

			PatchResult result;
			try {
				result = Exec.ApplyPatches(removeTurboSkip, patchAudioThread, audioThreadDivisor, patchBgmQueueing, patchAssets);
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
			new TextDisplayForm("Asset fix details for Cold Steel 2", Exec.AssetPatchDescriptions).ShowDialog();
		}
	}
}
