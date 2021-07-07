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
		private FileStorage Storage;

		public Sen2Form(string path, FileStorage storage) {
			SenLib.Logging.Log(string.Format("Initializing CS2 GUI for patching at {0}.", path));
			Path = path;
			Storage = storage;

			InitializeComponent();
			labelFile.Text = path;

			int assetPatchCount = Sen2Mods.GetAssetMods().Count;
			checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Patching CS2.");
				bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
				bool patchAudioThread = checkBoxPatchAudioThread.Checked;
				int audioThreadDivisor = 1000;
				bool fixVoiceTables = checkBoxFixVoiceFileLang.Checked;
				bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;
				bool patchAssets = checkBoxAssetPatches.Checked;
				bool disableMouseCapture = checkBoxDisableMouseCam.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;

				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					removeTurboSkip: removeTurboSkip,
					patchAudioThread: patchAudioThread,
					audioThreadDivisor: audioThreadDivisor,
					patchBgmQueueing: patchBgmQueueing,
					correctLanguageVoiceTables: fixVoiceTables,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss
				));
				if (patchAssets) {
					mods.AddRange(Sen2Mods.GetAssetMods());
				}

				GamePatchClass.RunPatch(new GamePatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS2.");
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods());
				mods.AddRange(Sen2Mods.GetAssetMods());
				GameUnpatchClass.RunUnpatch(new GameUnpatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 2", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen2Mods.GetAssetMods())).ShowDialog();
		}
	}
}
