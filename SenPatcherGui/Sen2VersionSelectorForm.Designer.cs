
namespace SenPatcherGui {
	partial class Sen2VersionSelectorForm {
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing) {
			if (disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent() {
			this.label1 = new System.Windows.Forms.Label();
			this.radioButtonV140 = new System.Windows.Forms.RadioButton();
			this.radioButtonV141 = new System.Windows.Forms.RadioButton();
			this.radioButtonV142 = new System.Windows.Forms.RadioButton();
			this.buttonOK = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 9);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(204, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Please select the game version to restore:";
			// 
			// radioButtonV140
			// 
			this.radioButtonV140.AutoSize = true;
			this.radioButtonV140.Location = new System.Drawing.Point(16, 29);
			this.radioButtonV140.Name = "radioButtonV140";
			this.radioButtonV140.Size = new System.Drawing.Size(78, 17);
			this.radioButtonV140.TabIndex = 1;
			this.radioButtonV140.Text = "Version 1.4";
			this.radioButtonV140.UseVisualStyleBackColor = true;
			// 
			// radioButtonV141
			// 
			this.radioButtonV141.AutoSize = true;
			this.radioButtonV141.Location = new System.Drawing.Point(16, 52);
			this.radioButtonV141.Name = "radioButtonV141";
			this.radioButtonV141.Size = new System.Drawing.Size(87, 17);
			this.radioButtonV141.TabIndex = 2;
			this.radioButtonV141.Text = "Version 1.4.1";
			this.radioButtonV141.UseVisualStyleBackColor = true;
			// 
			// radioButtonV142
			// 
			this.radioButtonV142.AutoSize = true;
			this.radioButtonV142.Checked = true;
			this.radioButtonV142.Location = new System.Drawing.Point(16, 75);
			this.radioButtonV142.Name = "radioButtonV142";
			this.radioButtonV142.Size = new System.Drawing.Size(87, 17);
			this.radioButtonV142.TabIndex = 3;
			this.radioButtonV142.TabStop = true;
			this.radioButtonV142.Text = "Version 1.4.2";
			this.radioButtonV142.UseVisualStyleBackColor = true;
			// 
			// buttonOK
			// 
			this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonOK.Location = new System.Drawing.Point(73, 100);
			this.buttonOK.Name = "buttonOK";
			this.buttonOK.Size = new System.Drawing.Size(75, 23);
			this.buttonOK.TabIndex = 4;
			this.buttonOK.Text = "OK";
			this.buttonOK.UseVisualStyleBackColor = true;
			this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
			// 
			// buttonCancel
			// 
			this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCancel.Location = new System.Drawing.Point(154, 100);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.Size = new System.Drawing.Size(75, 23);
			this.buttonCancel.TabIndex = 5;
			this.buttonCancel.Text = "Cancel";
			this.buttonCancel.UseVisualStyleBackColor = true;
			this.buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
			// 
			// Sen2VersionSelectorForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(241, 135);
			this.Controls.Add(this.buttonCancel);
			this.Controls.Add(this.buttonOK);
			this.Controls.Add(this.radioButtonV142);
			this.Controls.Add(this.radioButtonV141);
			this.Controls.Add(this.radioButtonV140);
			this.Controls.Add(this.label1);
			this.Name = "Sen2VersionSelectorForm";
			this.Text = "Select Version";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.RadioButton radioButtonV140;
		private System.Windows.Forms.RadioButton radioButtonV141;
		private System.Windows.Forms.RadioButton radioButtonV142;
		private System.Windows.Forms.Button buttonOK;
		private System.Windows.Forms.Button buttonCancel;
	}
}