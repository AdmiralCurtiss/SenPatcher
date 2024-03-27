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
		private FileStorage Storage;

		public Sen4Form(string path, FileStorage storage) {
			SenLib.Logging.Log(string.Format("Initializing CS4 GUI for patching at {0}.", path));
			Path = path;
			Storage = storage;

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
				bool allowNightmare = checkBoxAllowNightmare.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool confirmCancelOption = checkBoxButtonLayout.Checked;
				bool defaultJpLayout = confirmCancelOption && comboBoxButtonLayout.SelectedIndex == 1;

				// TODO: copy native DLL to target

				WriteToIni();
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void ReadFromIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile(inipath);
				checkBoxAssetPatches.Checked = ini.GetBool("CS4", "AssetFixes", true);
				checkBoxAllowNightmare.Checked = ini.GetBool("CS4", "AllowSwitchToNightmare", true);
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
				IniFile ini;
				try {
					ini = new IniFile(inipath);
				} catch (Exception) {
					ini = new IniFile();
				}
				ini.SetBool("CS4", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("CS4", "AllowSwitchToNightmare", checkBoxAllowNightmare.Checked);
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

				// TODO: remove native DLL

			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
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
