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
		private FileStorage Storage;

		public Sen1Form(string path) {
			Path = path;
			FilenameFix.FixupIncorrectEncodingInFilenames(path, 1);
			Storage = FileModExec.InitializeAndPersistFileStorage(path, Sen1KnownFiles.Files);

			InitializeComponent();
			labelFile.Text = path;

			int assetPatchCount = Sen1Mods.GetAssetMods().Count;
			checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";

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
				var mods = new List<FileMod>();
				mods.AddRange(Sen1Mods.GetExecutableMods(
					removeTurboSkip: removeTurboSkip,
					allowR2NotebookShortcut: allowR2NotebookShortcut,
					turboKey: turboKey,
					fixTextureIds: fixTextureIds
				));
				if (patchAssets) {
					mods.AddRange(Sen1Mods.GetAssetMods());
				}
				result = FileModExec.ExecuteMods(Path, Storage, mods);
			} catch (Exception ex) {
				MessageBox.Show("Exception occurred: " + ex.ToString());
				return;
			}

			if (result.AllSuccessful) {
				MessageBox.Show("Patch successful.\n\nA backup has been created at " + System.IO.Path.Combine(Path, "senpatcher_rerun_revert_data.bin") + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
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
				var mods = new List<FileMod>();
				mods.AddRange(Sen1Mods.GetExecutableMods());
				mods.AddRange(Sen1Mods.GetAssetMods());
				result = FileModExec.RevertMods(Path, Storage, mods);
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
			new TextDisplayForm("Asset fix details for Cold Steel 1", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen1Mods.GetAssetMods())).ShowDialog();
		}
	}
}
