﻿using HyoutaUtils;
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
	public partial class TXForm : Form {
		private string Path;
		private static string RelativeDllPath = "DINPUT8.dll";

		public TXForm(string path) {
			SenLib.Logging.Log(string.Format("Initializing TX GUI for patching at {0}.", path));
			Path = path;

			InitializeComponent();
			buttonAssetFixDetails.Visible = false;
			labelFile.Text = path;
			comboBoxGameLanguage.Items.Add("Japanese / 日本語");
			comboBoxGameLanguage.Items.Add("English / 英語");

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching TX.");
				WriteToIni();

				string dllpath = System.IO.Path.Combine(Path, RelativeDllPath);
				File.Copy(System.IO.Path.Combine("Tokyo Xanadu eX+", RelativeDllPath), dllpath, true);
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
				comboBoxGameLanguage.SelectedIndex = (ini.GetString("TX", "Language", "English").ToLowerInvariant() == "japanese") ? 0 : 1;
				checkBoxSkipLogos.Checked = ini.GetBool("TX", "SkipLogos", true);
				checkBoxSkipMovies.Checked = ini.GetBool("TX", "SkipAllMovies", false);
			} catch (Exception) { }
		}
		private void WriteToIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile();
				try {
					ini.LoadIniFromString(System.IO.File.ReadAllText(inipath), overwriteExistingValues: false);
				} catch (Exception) { }
				ini.LoadIniFromString(Properties.Resources.senpatcher_settings_tx, overwriteExistingValues: true);
				ini.SetString("TX", "Language", comboBoxGameLanguage.SelectedIndex == 0 ? "Japanese" : "English");
				ini.SetBool("TX", "SkipLogos", checkBoxSkipLogos.Checked);
				ini.SetBool("TX", "SkipAllMovies", checkBoxSkipMovies.Checked);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching TX.");
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
			new TextDisplayForm("Asset fix details for Tokyo Xanadu", "(details currently not available)").ShowDialog();
		}
	}
}