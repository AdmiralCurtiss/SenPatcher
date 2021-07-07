using HyoutaUtils;
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
	public partial class Sen2SystemDataForm : Form {
		private class ButtonMapping {
			public int InGameId;
			public string DisplayString;

			public ButtonMapping(int id, string str) {
				this.InGameId = id;
				this.DisplayString = str;
			}

			public override string ToString() {
				return DisplayString;
			}
		}

		private ButtonMapping LeaveUnchanged = new ButtonMapping(-1, "(invalid value)");
		private ButtonMapping ZoomIn = new ButtonMapping(0, "Zoom In");
		private ButtonMapping ZoomOut = new ButtonMapping(1, "Zoom Out");
		private ButtonMapping ZoomInOut = new ButtonMapping(2, "Zoom In/Out");
		private ButtonMapping ResetCamera = new ButtonMapping(3, "Reset Camera");
		private ButtonMapping TurnCharacter = new ButtonMapping(4, "Turn Character");
		private ButtonMapping ChangeLeaderNext = new ButtonMapping(5, "Change Leader (Next)");
		private ButtonMapping ChangeLeaderPrevious = new ButtonMapping(6, "Change Leader (Previous)");
		private ButtonMapping Dash = new ButtonMapping(7, "Dash");
		private ButtonMapping Walk = new ButtonMapping(8, "Walk");

		private Sen2SystemData SystemData;
		private string SystemDataPath;
		private EndianUtils.Endianness Endian;

		public Sen2SystemDataForm(Sen2SystemData data, string path, EndianUtils.Endianness endian) {
			SenLib.Logging.Log("Initializing CS2 System Data edit GUI.");
			this.SystemData = data;
			this.SystemDataPath = path;
			this.Endian = endian;

			InitializeComponent();


			InitButtonMappingCombobox(comboBoxDPadUp, data.DPadUpButtonMapping, ZoomIn);
			InitButtonMappingCombobox(comboBoxDPadDown, data.DPadDownButtonMapping, ZoomOut);
			InitButtonMappingCombobox(comboBoxDPadLeft, data.DPadLeftButtonMapping, TurnCharacter);
			InitButtonMappingCombobox(comboBoxDPadRight, data.DPadRightButtonMapping, TurnCharacter);
			InitButtonMappingCombobox(comboBoxL1, data.L1ButtonMapping, ChangeLeaderNext, false);
			InitButtonMappingCombobox(comboBoxR1, data.R1ButtonMapping, Dash, false);
			InitButtonMappingCombobox(comboBoxCircle, data.CircleButtonMapping, Walk, false);
		}

		private void InitButtonMappingComboboxSingle(ComboBox combobox, ButtonMapping mapping, ButtonMapping defaultMapping) {
			if (mapping.InGameId == defaultMapping.InGameId) {
				combobox.Items.Add(new ButtonMapping(mapping.InGameId, mapping.DisplayString + " (default)"));
			} else {
				combobox.Items.Add(mapping);
			}
		}

		private void InitButtonMappingCombobox(ComboBox combobox, ushort value, ButtonMapping defaultMapping, bool allowTurnCharacter = true) {
			InitButtonMappingComboboxSingle(combobox, ZoomIn, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, ZoomOut, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, ZoomInOut, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, ResetCamera, defaultMapping);
			if (allowTurnCharacter) {
				InitButtonMappingComboboxSingle(combobox, TurnCharacter, defaultMapping);
			}
			InitButtonMappingComboboxSingle(combobox, ChangeLeaderNext, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, ChangeLeaderPrevious, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, Dash, defaultMapping);
			InitButtonMappingComboboxSingle(combobox, Walk, defaultMapping);

			bool foundValue = false;
			foreach (ButtonMapping bm in combobox.Items) {
				if (bm.InGameId == ((int)value)) {
					combobox.SelectedItem = bm;
					foundValue = true;
					break;
				}
			}

			if (!foundValue) {
				combobox.Items.Add(LeaveUnchanged);
				combobox.SelectedItem = LeaveUnchanged;
			}
		}

		private void WriteBackButtonMapping(ButtonMapping mapping, ref ushort value) {
			if (mapping.InGameId == -1) {
				return;
			}
			value = (ushort)mapping.InGameId;
		}

		private void buttonSave_Click(object sender, EventArgs e) {
			WriteBackButtonMapping(comboBoxDPadUp.SelectedItem as ButtonMapping, ref SystemData.DPadUpButtonMapping);
			WriteBackButtonMapping(comboBoxDPadDown.SelectedItem as ButtonMapping, ref SystemData.DPadDownButtonMapping);
			WriteBackButtonMapping(comboBoxDPadLeft.SelectedItem as ButtonMapping, ref SystemData.DPadLeftButtonMapping);
			WriteBackButtonMapping(comboBoxDPadRight.SelectedItem as ButtonMapping, ref SystemData.DPadRightButtonMapping);
			WriteBackButtonMapping(comboBoxL1.SelectedItem as ButtonMapping, ref SystemData.L1ButtonMapping);
			WriteBackButtonMapping(comboBoxR1.SelectedItem as ButtonMapping, ref SystemData.R1ButtonMapping);
			WriteBackButtonMapping(comboBoxCircle.SelectedItem as ButtonMapping, ref SystemData.CircleButtonMapping);

			SenLib.Logging.Log(string.Format("Saving CS2 system data to {0}", SystemDataPath));
			using (var fs = new FileStream(SystemDataPath, FileMode.Create, FileAccess.Write)) {
				SystemData.SerializeToStream(fs, Endian);
			}
			MessageBox.Show("Saved!");
			Close();
		}
	}
}
