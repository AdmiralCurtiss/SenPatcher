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
	public partial class TextDisplayForm : Form {
		public TextDisplayForm(string name, string text) {
			SenLib.Logging.Log("Initializing text display window.");
			InitializeComponent();
			this.Text = name;
			this.textBox1.Text = text;
			this.textBox1.SelectedText = "";
		}

		private void buttonClose_Click(object sender, EventArgs e) {
			Close();
		}

		private void TextDisplayForm_Load(object sender, EventArgs e) {
			this.ActiveControl = buttonClose;
		}
	}
}
