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
	public partial class Sen5Form : Form {
		private string Path;
		private static string RelativeDllPath = "bin\\Win64\\DSOUND.dll";

		public Sen5Form(string path) {
			SenLib.Logging.Log(string.Format("Initializing Reverie GUI for patching at {0}.", path));
			Path = path;

			InitializeComponent();
			labelFile.Text = path;

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching Reverie.");
				WriteToIni();

				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				File.Copy(System.IO.Path.Combine("The Legend of Heroes Trails into Reverie", RelativeDllPath), dllpath, true);
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
				checkBoxAssetPatches.Checked = ini.GetBool("Reverie", "AssetFixes", true);
				checkBoxBgmEnqueueingLogic.Checked = ini.GetBool("Reverie", "FixBgmEnqueue", true);
				checkBoxDisableMouseCam.Checked = ini.GetBool("Reverie", "DisableMouseCapture", false);
				checkBoxShowMouseCursor.Checked = ini.GetBool("Reverie", "ShowMouseCursor", false);
				checkBoxDisableFpsLimitOnFocusLoss.Checked = ini.GetBool("Reverie", "DisableFpsLimitOnFocusLoss", false);
			} catch (Exception) { }
		}
		private void WriteToIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile();
				try {
					ini.LoadIniFromString(System.IO.File.ReadAllText(inipath), overwriteExistingValues: false);
				} catch (Exception) { }
				ini.LoadIniFromString(Properties.Resources.senpatcher_settings_reverie, overwriteExistingValues: true);
				ini.SetBool("Reverie", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("Reverie", "FixBgmEnqueue", checkBoxBgmEnqueueingLogic.Checked);
				ini.SetBool("Reverie", "DisableMouseCapture", checkBoxDisableMouseCam.Checked);
				ini.SetBool("Reverie", "ShowMouseCursor", checkBoxShowMouseCursor.Checked);
				ini.SetBool("Reverie", "DisableFpsLimitOnFocusLoss", checkBoxDisableFpsLimitOnFocusLoss.Checked);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching Reverie.");
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
			new TextDisplayForm("Asset fix details for Reverie", "(details currently not available)").ShowDialog();
		}
	}
}
