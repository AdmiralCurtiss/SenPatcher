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

			int assetPatchCount = Sen2Mods.GetAssetMods(Sen2Version.v142).Count;
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
				bool showMouseCursor = checkBoxShowMouseCursor.Checked;
				bool disablePauseOnFocusLoss = checkBoxDisablePauseOnFocusLoss.Checked;
				bool fixArtsSupport = checkBoxArtsSupport.Checked;
				bool fixControllerMapping = checkBoxControllerMapping.Checked;
				bool force0Kerning = checkBoxForce0Kerning.Checked;

				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					Sen2Version.v142,
					removeTurboSkip: removeTurboSkip,
					patchAudioThread: patchAudioThread,
					audioThreadDivisor: audioThreadDivisor,
					patchBgmQueueing: patchBgmQueueing,
					correctLanguageVoiceTables: fixVoiceTables,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixControllerMapping: fixControllerMapping,
					fixArtsSupport: fixArtsSupport,
					force0Kerning: force0Kerning
				));
				if (patchAssets) {
					mods.AddRange(Sen2Mods.GetAssetMods(Sen2Version.v142));
				}

				GamePatchClass.RunPatch(new GamePatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			try {
				SenLib.Logging.Log("Unpatching CS2.");

				bool canUnpatchTo140 = Sen2Mods.CanRevertTo(Sen2Version.v14, Storage);
				bool canUnpatchTo141 = Sen2Mods.CanRevertTo(Sen2Version.v141, Storage);
				bool canUnpatchTo142 = Sen2Mods.CanRevertTo(Sen2Version.v142, Storage);

				Sen2Version version = Sen2Version.v142;
				if (!canUnpatchTo140 && canUnpatchTo141 && canUnpatchTo142) {
					// this is the typical case if you start with 1.4.1, which is the steam/gog default version
					// just unpatch to 1.4.1 here and don't bother asking, the difference between the two is insignificant anyway
					version = Sen2Version.v141;
				} else if (!canUnpatchTo140 && !canUnpatchTo141 && canUnpatchTo142) {
					version = Sen2Version.v142;
				} else if (!canUnpatchTo140 && canUnpatchTo141 && !canUnpatchTo142) {
					version = Sen2Version.v141;
				} else if (canUnpatchTo140 && !canUnpatchTo141 && !canUnpatchTo142) {
					version = Sen2Version.v14;
				} else if (!canUnpatchTo140 && !canUnpatchTo141 && !canUnpatchTo142) {
					MessageBox.Show("Cannot unpatch to any version, not enough known data.");
					return;
				} else {
					var selector = new Sen2VersionSelectorForm(canUnpatchTo140, canUnpatchTo141, canUnpatchTo142);
					selector.ShowDialog();
					if (selector.Success) {
						version = selector.Version;
					} else {
						return;
					}
				}

				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(version));
				mods.AddRange(Sen2Mods.GetAssetMods(version));
				GameUnpatchClass.RunUnpatch(new GameUnpatchClass(Path, Storage, mods));
			} catch (Exception ex) {
				MessageBox.Show("Unknown error occurred: " + ex.Message);
			}
		}

		private void buttonAssetFixDetails_Click(object sender, EventArgs e) {
			new TextDisplayForm("Asset fix details for Cold Steel 2", SenUtils.ExtractUserFriendlyStringFromModDescriptions(Sen2Mods.GetAssetMods(Sen2Version.v142))).ShowDialog();
		}
	}
}
