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

		public Sen1Form(string path, FileStorage storage) {
			SenLib.Logging.Log(string.Format("Initializing CS1 GUI for patching at {0}.", path));
			Path = path;
			Storage = storage;

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
			try {
				SenLib.Logging.Log("Patching CS1.");
				bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
				bool allowR2NotebookShortcut = checkBoxAllowR2InTurboMode.Checked;
				int turboKey = comboBoxTurboModeKey.SelectedIndex;
				bool fixTextureIds = checkBoxFixHdTextureId.Checked;
				bool fixVoiceTables = checkBoxFixVoiceFileLang.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool fixArtsSupport = checkBoxArtsSupport.Checked;
				bool force0Kerning = checkBoxForce0Kerning.Checked;
				bool forceXInput = checkBoxForceXInput.Checked;

				var mods = new List<FileMod>();
				mods.AddRange(Sen1Mods.GetExecutableMods(
					removeTurboSkip: removeTurboSkip,
					allowR2NotebookShortcut: allowR2NotebookShortcut,
					turboKey: turboKey,
					fixTextureIds: fixTextureIds,
					correctLanguageVoiceTables: fixVoiceTables,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixArtsSupport: fixArtsSupport,
					force0Kerning: force0Kerning,
					forceXInput: forceXInput
				));
				if (patchAssets) {
					mods.AddRange(Sen1Mods.GetAssetMods());
				}

				GamePatchClass.RunPatch(new GamePatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS1.");
				var mods = new List<FileMod>();
				mods.AddRange(Sen1Mods.GetExecutableMods());
				mods.AddRange(Sen1Mods.GetAssetMods());
				GameUnpatchClass.RunUnpatch(new GameUnpatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 1", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen1Mods.GetAssetMods())).ShowDialog();
		}
	}
}
