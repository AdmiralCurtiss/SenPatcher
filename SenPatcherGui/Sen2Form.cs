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

		public Sen2Form(string path) {
			Path = path;
			FilenameFix.FixupIncorrectEncodingInFilenames(path, 2);
			Storage = FileModExec.InitializeAndPersistFileStorage(path, Sen2KnownFiles.Files);

			InitializeComponent();
			labelFile.Text = path;

			int assetPatchCount = Sen2Mods.GetAssetMods().Count;
			checkBoxAssetPatches.Text += " (" + assetPatchCount + " file" + (assetPatchCount == 1 ? "" : "s") + ")";
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
			bool patchAudioThread = checkBoxPatchAudioThread.Checked;
			int audioThreadDivisor = 1000;
			bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;
			bool patchAssets = checkBoxAssetPatches.Checked;

			PatchResult result;
			try {
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					removeTurboSkip: removeTurboSkip,
					patchAudioThread: patchAudioThread,
					audioThreadDivisor: audioThreadDivisor,
					patchBgmQueueing: patchBgmQueueing
				));
				if (patchAssets) {
					mods.AddRange(Sen2Mods.GetAssetMods());
				}
				result = FileModExec.ExecuteMods(Path, Storage, mods);
			} catch (Exception ex) {
				MessageBox.Show("Exception occurred: " + ex.ToString());
				return;
			}

			if (result.AllSuccessful) {
				MessageBox.Show("Patch successful.\n\nA backup has been created at " + System.IO.Path.Combine(Path, "senpatcher_rerun_revert_data.bin") + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				return;
			} else {
				MessageBox.Show(
					  "Patching failed for " + result.FailedFiles + " file" + (result.FailedFiles == 1 ? "" : "s") + ".\n"
					+ "Verify that the game files are writable and not corrupted."
				);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			PatchResult result;
			try {
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods());
				mods.AddRange(Sen2Mods.GetAssetMods());
				result = FileModExec.RevertMods(Path, Storage, mods);
			} catch (Exception ex) {
				MessageBox.Show("Exception occurred: " + ex.ToString());
				return;
			}

			if (result.AllSuccessful) {
				MessageBox.Show("Original " + (result.SuccessfulFiles == 1 ? "file has" : "files have") + " been restored.");
				return;
			} else {
				MessageBox.Show(
					  "Restoration failed for " + result.FailedFiles + " file" + (result.FailedFiles == 1 ? "" : "s") + ".\n"
					+ "Verify that the game files are writable and not corrupted."
				);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 2", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen2Mods.GetAssetMods())).ShowDialog();
		}
	}
}
