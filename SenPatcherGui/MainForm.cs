using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
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
	public partial class MainForm : Form {
		public MainForm() {
			InitializeComponent();
		}

		private void buttonCs1SteamEn_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen1SteamDir, SenCommonPaths.Sen1EnExePath), new List<SenVersion>() { SenVersion.Sen1_v1_6_En });
		}

		private void buttonCs1SteamJp_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen1SteamDir, SenCommonPaths.Sen1JpExePath), new List<SenVersion>() { SenVersion.Sen1_v1_6_Jp });
		}

		private void buttonCs1GalaxyEn_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen1GalaxyDir, SenCommonPaths.Sen1EnExePath), new List<SenVersion>() { SenVersion.Sen1_v1_6_En });
		}

		private void buttonCs1GalaxyJp_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen1GalaxyDir, SenCommonPaths.Sen1JpExePath), new List<SenVersion>() { SenVersion.Sen1_v1_6_Jp });
		}

		private void buttonCs2SteamEn_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen2SteamDir, SenCommonPaths.Sen2EnExePath), new List<SenVersion>() { SenVersion.Sen2_v1_4_1_En, SenVersion.Sen2_v1_4_2_En });
		}

		private void buttonCs2SteamJp_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen2SteamDir, SenCommonPaths.Sen2JpExePath), new List<SenVersion>() { SenVersion.Sen2_v1_4_1_Jp, SenVersion.Sen2_v1_4_2_Jp });
		}

		private void buttonCs2GalaxyEn_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen2GalaxyDir, SenCommonPaths.Sen2EnExePath), new List<SenVersion>() { SenVersion.Sen2_v1_4_1_En, SenVersion.Sen2_v1_4_2_En });
		}

		private void buttonCs2GalaxyJp_Click(object sender, EventArgs e) {
			OpenFileGui(Path.Combine(SenCommonPaths.Sen2GalaxyDir, SenCommonPaths.Sen2JpExePath), new List<SenVersion>() { SenVersion.Sen2_v1_4_1_Jp, SenVersion.Sen2_v1_4_2_Jp });
		}

		private void buttonManuallySelect_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Filter = "Cold Steel executables (ed8*.exe)|ed8*.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenFileGui(d.FileName, null);
				}
			}
		}

		private void OpenFileGui(string path, List<SenVersion> expectedVersions) {
			if (!File.Exists(path)) {
				MessageBox.Show("No file found at " + path + ".");
				return;
			}

			Stream binary = null;
			SenVersion? actualVersion = null;
			try {
				(binary, actualVersion) = SenVersionIdentifier.OpenAndIdentifyGame(path);
			} catch (Exception ex) {
				MessageBox.Show("Error while identifying " + path + ": " + ex.ToString());
				return;
			}

			if (binary == null || actualVersion == null) {
				MessageBox.Show("Could not identify file at " + path + " as any supported Cold Steel executable.");
				return;
			}

			switch (actualVersion) {
				case SenVersion.Sen1_v1_0_En:
				case SenVersion.Sen1_v1_0_Jp:
				case SenVersion.Sen1_v1_1_En:
				case SenVersion.Sen1_v1_1_Jp:
				case SenVersion.Sen1_v1_2_1_En:
				case SenVersion.Sen1_v1_2_1_Jp:
				case SenVersion.Sen1_v1_3_En:
				case SenVersion.Sen1_v1_3_Jp:
				case SenVersion.Sen1_v1_3_5_En:
				case SenVersion.Sen1_v1_3_5_Jp:
				case SenVersion.Sen1_v1_4_En:
				case SenVersion.Sen1_v1_4_Jp:
				case SenVersion.Sen1_v1_5_En:
				case SenVersion.Sen1_v1_5_Jp:
					MessageBox.Show("Your Cold Steel game is an older version than the ones supported by this patcher. Please update to version 1.6.");
					break;
				case SenVersion.Sen1_v1_6_En:
				case SenVersion.Sen1_v1_6_Jp:
					new Sen1Form(path, binary, actualVersion.Value).ShowDialog();
					break;
				case SenVersion.Sen1Launcher_v1_0:
				case SenVersion.Sen1Launcher_v1_1:
				case SenVersion.Sen1Launcher_v1_2_1:
				case SenVersion.Sen1Launcher_v1_3_to_v1_5:
				case SenVersion.Sen1Launcher_v1_6:
					MessageBox.Show("Please select ed8.exe or ed8jp.exe instead of Sen1Launcher.exe.");
					break;
				case SenVersion.Sen2_v1_0_En:
				case SenVersion.Sen2_v1_0_Jp:
				case SenVersion.Sen2_v1_1_En:
				case SenVersion.Sen2_v1_1_Jp:
				case SenVersion.Sen2_v1_2_En:
				case SenVersion.Sen2_v1_2_Jp:
				case SenVersion.Sen2_v1_3_En:
				case SenVersion.Sen2_v1_3_Jp:
				case SenVersion.Sen2_v1_3_1_En:
				case SenVersion.Sen2_v1_3_1_Jp:
				case SenVersion.Sen2_v1_4_En:
				case SenVersion.Sen2_v1_4_Jp:
					MessageBox.Show("Your Cold Steel II game is an older version than the ones supported by this patcher. Please update to at least version 1.4.1.");
					break;
				case SenVersion.Sen2_v1_4_1_En:
				case SenVersion.Sen2_v1_4_1_Jp:
				case SenVersion.Sen2_v1_4_2_En:
				case SenVersion.Sen2_v1_4_2_Jp:
					new Sen2Form(path, binary, actualVersion.Value).ShowDialog();
					break;
				case SenVersion.Sen2Launcher_v1_0:
				case SenVersion.Sen2Launcher_v1_1:
				case SenVersion.Sen2Launcher_v1_2_to_v1_3_1:
				case SenVersion.Sen2Launcher_v1_4_to_v1_4_2:
					MessageBox.Show("Please select ed8_2_PC_US.exe or ed8_2_PC_JP.exe (in the bin/Win32 directory) instead of Sen2Launcher.exe.");
					break;
				default:
					MessageBox.Show("Internal error?");
					break;
			}
		}

		private void buttonCs1SystemDataAuto_Click(object sender, EventArgs e) {
			OpenCs1SystemData(SenCommonPaths.Sen1SystemDataFile);
		}

		private void buttonCs1SystemDataManual_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Filter = "CS1 System Data File (" + SenCommonPaths.Sen1SystemDataFilename + ")|" + SenCommonPaths.Sen1SystemDataFilename + "|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs1SystemData(d.FileName);
				}
			}
		}

		private void OpenCs1SystemData(string path) {
			if (!File.Exists(path)) {
				MessageBox.Show("No file found at " + path + ".");
				return;
			}

			Sen1SystemData data = null;
			using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
				if (fs.Length != Sen1SystemData.FileLength) {
					MessageBox.Show("Incorrect filesize for CS1 system data at " + path + ".");
					return;
				}
				data = new Sen1SystemData(fs.CopyToMemory(), HyoutaUtils.EndianUtils.Endianness.LittleEndian);
			}

			new Sen1SystemDataForm(data, path, HyoutaUtils.EndianUtils.Endianness.LittleEndian).ShowDialog();
		}

		private void buttonCs2SystemDataAuto_Click(object sender, EventArgs e) {
			OpenCs2SystemData(SenCommonPaths.Sen2SystemDataFile);
		}

		private void buttonCs2SystemDataManual_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Filter = "CS2 System Data File (" + SenCommonPaths.Sen2SystemDataFilename + ")|" + SenCommonPaths.Sen2SystemDataFilename + "|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs2SystemData(d.FileName);
				}
			}
		}

		private void OpenCs2SystemData(string path) {
			if (!File.Exists(path)) {
				MessageBox.Show("No file found at " + path + ".");
				return;
			}

			Sen2SystemData data = null;
			using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
				if (fs.Length != Sen2SystemData.FileLength) {
					MessageBox.Show("Incorrect filesize for CS2 system data at " + path + ".");
					return;
				}
				data = new Sen2SystemData(fs.CopyToMemory(), HyoutaUtils.EndianUtils.Endianness.LittleEndian);
			}

			new Sen2SystemDataForm(data, path, HyoutaUtils.EndianUtils.Endianness.LittleEndian).ShowDialog();
		}
	}
}
