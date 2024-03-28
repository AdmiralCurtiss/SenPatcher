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
		private static string RelativeDllPath = "DINPUT8.dll";

		public Sen1Form(string path) {
			SenLib.Logging.Log(string.Format("Initializing CS1 GUI for patching at {0}.", path));
			Path = path;

			InitializeComponent();
			labelFile.Text = path;

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

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS1.");
				bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
				bool allowR2NotebookShortcut = checkBoxAllowR2InTurboMode.Checked;
				int turboKey = comboBoxTurboModeKey.SelectedIndex;
				bool fixVoiceTables = checkBoxFixVoiceFileLang.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool fixArtsSupport = checkBoxArtsSupport.Checked;
				bool force0Kerning = checkBoxForce0Kerning.Checked;
				bool forceXInput = checkBoxForceXInput.Checked;
				WriteToIni();

				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				File.Copy(System.IO.Path.Combine("Trails of Cold Steel", RelativeDllPath), dllpath, true);
				MessageBox.Show("SenPatcher DLL successfully copied to game directory.");
			} catch (Exception ex) {
				MessageBox.Show("Error occurred: " + ex.Message);
			}
		}

		private void ReadFromIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile();
				ini.LoadIniFromString(System.IO.File.ReadAllText(inipath), overwriteExistingValues: false);
				checkBoxAssetPatches.Checked = ini.GetBool("CS1", "AssetFixes", true);
				checkBoxBattleAutoSkip.Checked = ini.GetBool("CS1", "RemoveTurboSkip", true);
				checkBoxFixVoiceFileLang.Checked = ini.GetBool("CS1", "CorrectLanguageVoiceTables", true);
				checkBoxArtsSupport.Checked = ini.GetBool("CS1", "FixArtsSupportCutin", true);
				checkBoxForce0Kerning.Checked = ini.GetBool("CS1", "Force0Kerning", false);
				checkBoxDisableMouseCam.Checked = ini.GetBool("CS1", "DisableMouseCapture", false);
				checkBoxShowMouseCursor.Checked = ini.GetBool("CS1", "ShowMouseCursor", false);
				checkBoxDisablePauseOnFocusLoss.Checked = ini.GetBool("CS1", "DisablePauseOnFocusLoss", false);
				checkBoxForceXInput.Checked = ini.GetBool("CS1", "ForceXInput", false);
				checkBoxAllowR2InTurboMode.Checked = ini.GetBool("CS1", "AlwaysUseNotebookR2", false);
				int turboIndex = ini.GetInt("CS1", "TurboModeButton", 7);
				if (turboIndex >= 0 && turboIndex <= comboBoxTurboModeKey.Items.Count) {
					comboBoxTurboModeKey.SelectedIndex = turboIndex;
				}
			} catch (Exception) { }
		}
		private void WriteToIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile();
				try {
					ini.LoadIniFromString(System.IO.File.ReadAllText(inipath), overwriteExistingValues: false);
				} catch (Exception) { }
				ini.LoadIniFromString(Properties.Resources.senpatcher_settings_cs1, overwriteExistingValues: true);
				ini.SetBool("CS1", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("CS1", "RemoveTurboSkip", checkBoxBattleAutoSkip.Checked);
				ini.SetBool("CS1", "CorrectLanguageVoiceTables", checkBoxFixVoiceFileLang.Checked);
				ini.SetBool("CS1", "FixArtsSupportCutin", checkBoxArtsSupport.Checked);
				ini.SetBool("CS1", "Force0Kerning", checkBoxForce0Kerning.Checked);
				ini.SetBool("CS1", "DisableMouseCapture", checkBoxDisableMouseCam.Checked);
				ini.SetBool("CS1", "ShowMouseCursor", checkBoxShowMouseCursor.Checked);
				ini.SetBool("CS1", "DisablePauseOnFocusLoss", checkBoxDisablePauseOnFocusLoss.Checked);
				ini.SetBool("CS1", "ForceXInput", checkBoxForceXInput.Checked);
				ini.SetBool("CS1", "AlwaysUseNotebookR2", checkBoxAllowR2InTurboMode.Checked);
				ini.SetInt("CS1", "TurboModeButton", comboBoxTurboModeKey.SelectedIndex);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS1.");
				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				if (File.Exists(dllpath)) {
					System.IO.File.Delete(dllpath);
					MessageBox.Show("Removed SenPatcher. Files for installed mods have not been removed, but will no longer be loaded. To remove them, navigate to the game directory and delete the 'mods' folder.");
				} else {
					MessageBox.Show("Could not find anything to remove.");
				}
			} catch (Exception ex) {
				MessageBox.Show("Error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 1", "(details currently not available)").ShowDialog();
		}
	}
}
