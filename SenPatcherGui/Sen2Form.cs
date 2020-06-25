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
		private Sen2PatchExec Exec;

		public Sen2Form(string path, Stream binary, SenVersion version) {
			Exec = new Sen2PatchExec(path, binary, version);

			InitializeComponent();
			labelFile.Text = path;
			labelVersion.Text = Exec.HumanReadableVersion;
		}

		private void buttonPatch_Click(object sender, EventArgs e) {
			bool removeTurboSkip = checkBoxBattleAutoSkip.Checked;
			bool patchAudioThread = checkBoxPatchAudioThread.Checked;
			int audioThreadDivisor = patchAudioThread ? (int)numericUpDownTicksPerSecond.Value : 1000;
			bool patchBgmQueueing = checkBoxBgmEnqueueingLogic.Checked;

			if (Exec.ApplyPatches(removeTurboSkip, patchAudioThread, audioThreadDivisor, patchBgmQueueing)) {
				MessageBox.Show("File successfully patched.\n\nA backup has been created at " + (Exec.Path + SenCommonPaths.BackupPostfix) + ". Please do not delete this backup, as it can be used to revert the changes and/or re-run this patcher or a future version of the patcher.");
				Close();
				return;
			}
		}

		private void checkBoxPatchAudioThread_CheckedChanged(object sender, EventArgs e) {
			numericUpDownTicksPerSecond.Enabled = checkBoxPatchAudioThread.Checked;
		}

		private void buttonUnpatch_Click(object sender, EventArgs e) {
			if (Exec.RestoreOriginalFiles()) {
				MessageBox.Show("Original executable has been restored.");
				return;
			}
		}
	}
}
