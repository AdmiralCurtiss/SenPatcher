﻿using HyoutaUtils;
using SenLib;
using SenLib.Sen4;
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
	public partial class Sen4Form : Form {
		private string Path;
		private FileStorage Storage;

		public Sen4Form(string path, FileStorage storage) {
			SenLib.Logging.Log(string.Format("Initializing CS4 GUI for patching at {0}.", path));
			Path = path;
			Storage = storage;

			InitializeComponent();
			labelFile.Text = path;

			int assetPatchCount = Sen4Mods.GetAssetMods().Count;
			checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";

			comboBoxButtonLayout.Items.Clear();
			comboBoxButtonLayout.Items.Add("Xbox or English PlayStation Style (Confirm on bottom, Cancel on right)");
			comboBoxButtonLayout.Items.Add("Nintendo or Japanese PlayStation Style (Confirm on right, Cancel on bottom)");
			comboBoxButtonLayout.SelectedIndex = 1;
			comboBoxButtonLayout.Enabled = checkBoxButtonLayout.Checked;
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS4.");
				bool allowNightmare = checkBoxAllowNightmare.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool confirmCancelOption = checkBoxButtonLayout.Checked;
				bool defaultJpLayout = confirmCancelOption && comboBoxButtonLayout.SelectedIndex == 1;

				var mods = new List<FileMod>();
				mods.AddRange(Sen4Mods.GetExecutableMods(
					allowSwitchToNightmare: allowNightmare,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					separateSwapConfirmCancelOption: confirmCancelOption,
					defaultSwapConfirmCancelOptionOn: defaultJpLayout,
					fixSwappedButtonsWhenDynamicPromptsOff: true
				));
				if (patchAssets) {
					mods.AddRange(Sen4Mods.GetAssetMods(
						allowSwitchToNightmare: allowNightmare
					));
				}

				GamePatchClass.RunPatch(new GamePatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS4.");
				var mods = new List<FileMod>();
				mods.AddRange(Sen4Mods.GetExecutableMods());
				mods.AddRange(Sen4Mods.GetAssetMods());
				GameUnpatchClass.RunUnpatch(new GameUnpatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 4", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen4Mods.GetAssetMods())).ShowDialog();
		}

		private void checkBoxButtonLayout_CheckedChanged(object sender, EventArgs e) {
			comboBoxButtonLayout.Enabled = checkBoxButtonLayout.Checked;
		}
	}
}
