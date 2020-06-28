using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
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
	public partial class Sen1Form : Form {
		private Sen1PatchExec Exec;

		public Sen1Form(string path, Stream binary, SenVersion version) {
			Exec = new Sen1PatchExec(path, binary, version);

			InitializeComponent();
			labelFile.Text = Exec.Path;
			labelVersion.Text = Exec.HumanReadableVersion;

			comboBoxTurboModeKey.Items.Clear();
			comboBoxTurboModeKey.Items.Add("Square / X");
			comboBoxTurboModeKey.Items.Add("Cross / A");
			comboBoxTurboModeKey.Items.Add("Circle / B");
			comboBoxTurboModeKey.Items.Add("Triangle / Y");
			comboBoxTurboModeKey.Items.Add("L1 / LB");
			comboBoxTurboModeKey.Items.Add("R1 / RB");
			comboBoxTurboModeKey.Items.Add("L2 / LT");
			comboBoxTurboModeKey.Items.Add("R2 / RT");
			comboBoxTurboModeKey.Items.Add("Select / Back");
			comboBoxTurboModeKey.Items.Add("Start");
			comboBoxTurboModeKey.Items.Add("L3 / LS");
			comboBoxTurboModeKey.Items.Add("R3 / RS");
			comboBoxTurboModeKey.Items.Add("D-Pad Up");
			comboBoxTurboModeKey.Items.Add("D-Pad Right");
			comboBoxTurboModeKey.Items.Add("D-Pad Down");
			comboBoxTurboModeKey.Items.Add("D-Pad Left");
			comboBoxTurboModeKey.SelectedIndex = 7;
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
			bool allowR2NotebookShortcut = checkBoxAllowR2InTurboMode.Checked;
			int turboKey = comboBoxTurboModeKey.SelectedIndex;
			bool fixTextureIds = checkBoxFixHdTextureId.Checked;

			if (Exec.ApplyPatches(removeTurboSkip, allowR2NotebookShortcut, turboKey, fixTextureIds, false).AllSuccessful) {
				MessageBox.Show("Patch successful.\n\nA backup has been created at " + Path.GetFullPath(Exec.BackupFolder) + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				Close();
				return;
			}
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			if (Exec.RestoreOriginalFiles().AllSuccessful) {
				MessageBox.Show("Original executable has been restored.");
				return;
			}
		}
	}
}
