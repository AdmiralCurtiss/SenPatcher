using HyoutaUtils;
using SenLib;
using SenLib.Sen3;
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
	public partial class Sen3Form : Form {
		private string Path;
		private FileStorage Storage;

		public Sen3Form(string path, FileStorage storage) {
			SenLib.Logging.Log(string.Format("Initializing CS3 GUI for patching at {0}.", path));
			Path = path;
			Storage = storage;

			InitializeComponent();
			labelFile.Text = path;

			int assetPatchCount = Sen3Mods.GetAssetMods().Count;
			checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";

			ReadFromIni();
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS3.");
				bool fixButtonRemapping = checkBoxFixButtonRemapping.Checked;
				bool allowNightmare = checkBoxAllowNightmare.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool fixControllerMapping = checkBoxControllerMapping.Checked;
				bool forceXInput = checkBoxForceXInput.Checked;

				var mods = new List<FileMod>();
				mods.AddRange(Sen3Mods.GetExecutableMods(
					fixInGameButtonMappingValidity: fixButtonRemapping,
					allowSwitchToNightmare: allowNightmare,
					swapBrokenMasterQuartzValuesForDisplay: true,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixControllerMapping: fixControllerMapping,
					forceXInput: forceXInput
				));
				if (patchAssets) {
					mods.AddRange(Sen3Mods.GetAssetMods(
						allowSwitchToNightmare: allowNightmare
					));
				}

				GamePatchClass.RunPatch(new GamePatchClass(Path, Storage, mods));

				WriteToIni();
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void ReadFromIni() {
			try {
				string inipath = System.IO.Path.Combine(Path, "senpatcher_settings.ini");
				IniFile ini = new IniFile(inipath);
				checkBoxAssetPatches.Checked = ini.GetBool("CS3", "AssetFixes", true);
				checkBoxFixButtonRemapping.Checked = ini.GetBool("CS3", "FixInGameButtonRemapping", true);
				checkBoxAllowNightmare.Checked = ini.GetBool("CS3", "AllowSwitchToNightmare", true);
				checkBoxControllerMapping.Checked = ini.GetBool("CS3", "FixControllerMapping", true);
				checkBoxDisableMouseCam.Checked = ini.GetBool("CS3", "DisableMouseCapture", false);
				checkBoxShowMouseCursor.Checked = ini.GetBool("CS3", "ShowMouseCursor", false);
				checkBoxDisablePauseOnFocusLoss.Checked = ini.GetBool("CS3", "DisablePauseOnFocusLoss", false);
				checkBoxForceXInput.Checked = ini.GetBool("CS3", "ForceXInput", false);
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
				ini.SetBool("CS3", "AssetFixes", checkBoxAssetPatches.Checked);
				ini.SetBool("CS3", "FixInGameButtonRemapping", checkBoxFixButtonRemapping.Checked);
				ini.SetBool("CS3", "AllowSwitchToNightmare", checkBoxAllowNightmare.Checked);
				ini.SetBool("CS3", "FixControllerMapping", checkBoxControllerMapping.Checked);
				ini.SetBool("CS3", "DisableMouseCapture", checkBoxDisableMouseCam.Checked);
				ini.SetBool("CS3", "ShowMouseCursor", checkBoxShowMouseCursor.Checked);
				ini.SetBool("CS3", "DisablePauseOnFocusLoss", checkBoxDisablePauseOnFocusLoss.Checked);
				ini.SetBool("CS3", "ForceXInput", checkBoxForceXInput.Checked);
				ini.WriteToFile(inipath);
			} catch (Exception) { }
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS3.");
				var mods = new List<FileMod>();
				mods.AddRange(Sen3Mods.GetExecutableMods());
				mods.AddRange(Sen3Mods.GetAssetMods());
				GameUnpatchClass.RunUnpatch(new GameUnpatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 3", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen3Mods.GetAssetMods())).ShowDialog();
		}
	}
}
