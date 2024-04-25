using HyoutaUtils;
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
		private static string RelativeDllPath = "bin\\Win64\\DINPUT8.dll";

		public Sen4Form(string path) {
			SenLib.Logging.Log(string.Format("Initializing CS4 GUI for patching at {0}.", path));
			Path = path;

			InitializeComponent();
			labelFile.Text = path;

			comboBoxButtonLayout.Items.Clear();
			comboBoxButtonLayout.Items.Add("Xbox or English PlayStation Style (Confirm on bottom, Cancel on right)");
			comboBoxButtonLayout.Items.Add("Nintendo or Japanese PlayStation Style (Confirm on right, Cancel on bottom)");
			comboBoxButtonLayout.SelectedIndex = 1;
			comboBoxButtonLayout.Enabled = checkBoxButtonLayout.Checked;

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS4.");
				WriteToIni();

				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				File.Copy(System.IO.Path.Combine("The Legend of Heroes Trails of Cold Steel IV", RelativeDllPath), dllpath, true);
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
				checkBoxAssetPatches.Checked = ini.GetBool("CS4", "AssetFixes", true);
				checkBoxAllowNightmare.Checked = ini.GetBool("CS4", "AllowSwitchToNightmare", true);
				checkBoxBgmEnqueueingLogic.Checked = ini.GetBool("CS4", "FixBgmEnqueue", true);
				checkBoxButtonLayout.Checked = ini.GetBool("CS4", "ForceConfirmCancel", false);
				bool forceJp = ini.GetBool("CS4", "ForceConfirmJp", true);
				comboBoxButtonLayout.SelectedIndex = forceJp ? 1 : 0;
				checkBoxDisableMouseCam.Checked = ini.GetBool("CS4", "DisableMouseCapture", false);
				checkBoxShowMouseCursor.Checked = ini.GetBool("CS4", "ShowMouseCursor", false);
				checkBoxDisablePauseOnFocusLoss.Checked = ini.GetBool("CS4", "DisablePauseOnFocusLoss", false);
			} catch (Exception) { }
		}
		private void WriteToIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile();
				try {
					ini.LoadIniFromString(System.IO.File.ReadAllText(inipath), overwriteExistingValues: false);
				} catch (Exception) { }
				ini.LoadIniFromString(Properties.Resources.senpatcher_settings_cs4, overwriteExistingValues: true);
				ini.SetBool("CS4", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("CS4", "AllowSwitchToNightmare", checkBoxAllowNightmare.Checked);
				ini.SetBool("CS4", "FixBgmEnqueue", checkBoxBgmEnqueueingLogic.Checked);
				ini.SetBool("CS4", "ForceConfirmCancel", checkBoxButtonLayout.Checked);
				ini.SetBool("CS4", "ForceConfirmJp", comboBoxButtonLayout.SelectedIndex == 1);
				ini.SetBool("CS4", "DisableMouseCapture", checkBoxDisableMouseCam.Checked);
				ini.SetBool("CS4", "ShowMouseCursor", checkBoxShowMouseCursor.Checked);
				ini.SetBool("CS4", "DisablePauseOnFocusLoss", checkBoxDisablePauseOnFocusLoss.Checked);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS4.");
				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				if (File.Exists(dllpath)) {
					System.IO.File.Delete(dllpath);
					MessageBox.Show("Removed SenPatcher DLL. Files for installed mods have not been removed, but will no longer be loaded. To remove them, navigate to the game directory and delete the 'mods' folder.");
				} else {
					MessageBox.Show("Could not find anything to remove.");
				}
			} catch (Exception ex) {
				MessageBox.Show("Error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 4", "(details currently not available)").ShowDialog();
		}

		private void checkBoxButtonLayout_CheckedChanged(object sender, EventArgs e) {
			comboBoxButtonLayout.Enabled = checkBoxButtonLayout.Checked;
		}
	}
}
