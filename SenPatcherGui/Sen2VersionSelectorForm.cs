using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class Sen2VersionSelectorForm : Form {
		public bool Success = false;
		public SenLib.Sen2.Sen2Version Version = SenLib.Sen2.Sen2Version.v142;

		public Sen2VersionSelectorForm(bool has140, bool has141, bool has142) {
			InitializeComponent();

			if (!has140) {
				radioButtonV140.Enabled = false;
			}
			if (!has141) {
				radioButtonV141.Enabled = false;
			}
			if (!has142) {
				radioButtonV142.Enabled = false;
				if (has141) {
					radioButtonV141.Checked = true;
				} else {
					radioButtonV140.Checked = true;
				}
			}
		}

		private void buttonOK_Click(object sender, EventArgs e) {
			if (radioButtonV142.Checked) {
				Version = SenLib.Sen2.Sen2Version.v142;
			} else if (radioButtonV141.Checked) {
				Version = SenLib.Sen2.Sen2Version.v141;
			} else if (radioButtonV140.Checked) {
				Version = SenLib.Sen2.Sen2Version.v14;
			} else {
				return;
			}
			Success = true;
			Close();
		}

		private void buttonCancel_Click(object sender, EventArgs e) {
			Close();
		}
	}
}
