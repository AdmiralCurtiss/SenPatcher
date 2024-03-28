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
		private string Path;

		public Sen2Form(string path) {
			SenLib.Logging.Log(string.Format("Initializing CS2 GUI for patching at {0}.", path));
			Path = path;

			InitializeComponent();
			labelFile.Text = path;

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS2.");
				bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
				bool patchAudioThread = checkBoxPatchAudioThread.Checked;
				bool fixVoiceTables = checkBoxFixVoiceFileLang.Checked;
				bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool fixArtsSupport = checkBoxArtsSupport.Checked;
				bool fixControllerMapping = checkBoxControllerMapping.Checked;
				bool force0Kerning = checkBoxForce0Kerning.Checked;
				bool fixBattleScopeCrash = checkBoxFixBattleScopeCrash.Checked;
				bool forceXInput = checkBoxForceXInput.Checked;

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
				checkBoxAssetPatches.Checked = ini.GetBool("CS2", "AssetFixes", true);
				checkBoxBattleAutoSkip.Checked = ini.GetBool("CS2", "RemoveTurboSkip", true);
				checkBoxFixVoiceFileLang.Checked = ini.GetBool("CS2", "CorrectLanguageVoiceTables", true);
				checkBoxBgmEnqueueingLogic.Checked = ini.GetBool("CS2", "FixBgmEnqueue", true);
				checkBoxPatchAudioThread.Checked = ini.GetBool("CS2", "ReplaceAudioTimingThread", true);
				checkBoxControllerMapping.Checked = ini.GetBool("CS2", "FixControllerMapping", true);
				checkBoxArtsSupport.Checked = ini.GetBool("CS2", "FixArtsSupportCutin", true);
				checkBoxForce0Kerning.Checked = ini.GetBool("CS2", "Force0Kerning", false);
				checkBoxDisableMouseCam.Checked = ini.GetBool("CS2", "DisableMouseCapture", false);
				checkBoxShowMouseCursor.Checked = ini.GetBool("CS2", "ShowMouseCursor", false);
				checkBoxDisablePauseOnFocusLoss.Checked = ini.GetBool("CS2", "DisablePauseOnFocusLoss", false);
				checkBoxForceXInput.Checked = ini.GetBool("CS2", "ForceXInput", false);
				checkBoxFixBattleScopeCrash.Checked = ini.GetBool("CS2", "FixBattleScopeCrash", true);
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
				ini.SetBool("CS2", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("CS2", "RemoveTurboSkip", checkBoxBattleAutoSkip.Checked);
				ini.SetBool("CS2", "CorrectLanguageVoiceTables", checkBoxFixVoiceFileLang.Checked);
				ini.SetBool("CS2", "FixBgmEnqueue", checkBoxBgmEnqueueingLogic.Checked);
				ini.SetBool("CS2", "ReplaceAudioTimingThread", checkBoxPatchAudioThread.Checked);
				ini.SetBool("CS2", "FixControllerMapping", checkBoxControllerMapping.Checked);
				ini.SetBool("CS2", "FixArtsSupportCutin", checkBoxArtsSupport.Checked);
				ini.SetBool("CS2", "Force0Kerning", checkBoxForce0Kerning.Checked);
				ini.SetBool("CS2", "DisableMouseCapture", checkBoxDisableMouseCam.Checked);
				ini.SetBool("CS2", "ShowMouseCursor", checkBoxShowMouseCursor.Checked);
				ini.SetBool("CS2", "DisablePauseOnFocusLoss", checkBoxDisablePauseOnFocusLoss.Checked);
				ini.SetBool("CS2", "ForceXInput", checkBoxForceXInput.Checked);
				ini.SetBool("CS2", "FixBattleScopeCrash", checkBoxFixBattleScopeCrash.Checked);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS2.");

				// TODO: remove native DLL

			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 2", "(details currently not available)").ShowDialog();
		}
	}
}
