namespace SenPatcherGui {
	partial class Sen4Form {
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
			this.label2 = new System.Windows.Forms.Label();
			this.labelFile = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxAssetPatches = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.checkBoxAllowNightmare = new System.Windows.Forms.CheckBox();
			this.checkBoxDisableMouseCam = new System.Windows.Forms.CheckBox();
			this.checkBoxDisablePauseOnFocusLoss = new System.Windows.Forms.CheckBox();
			this.checkBoxButtonLayout = new System.Windows.Forms.CheckBox();
			this.comboBoxButtonLayout = new System.Windows.Forms.ComboBox();
			this.checkBoxShowMouseCursor = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(14, 114);
			this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(85, 15);
			this.label2.TabIndex = 3;
			this.label2.Text = "Patch Settings:";
			// 
			// labelFile
			// 
			this.labelFile.AutoSize = true;
			this.labelFile.Location = new System.Drawing.Point(66, 15);
			this.labelFile.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			this.labelFile.Name = "labelFile";
			this.labelFile.Size = new System.Drawing.Size(22, 15);
			this.labelFile.TabIndex = 1;
			this.labelFile.Text = "---";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(14, 15);
			this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(37, 15);
			this.label1.TabIndex = 0;
			this.label1.Text = "Path: ";
			// 
			// buttonPatch
			// 
			this.buttonPatch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonPatch.Location = new System.Drawing.Point(14, 275);
			this.buttonPatch.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.buttonPatch.Name = "buttonPatch";
			this.buttonPatch.Size = new System.Drawing.Size(595, 47);
			this.buttonPatch.TabIndex = 12;
			this.buttonPatch.Text = "Patch!";
			this.buttonPatch.UseVisualStyleBackColor = true;
			this.buttonPatch.Click += new System.EventHandler(this.buttonPatch_Click);
			// 
			// buttonUnpatch
			// 
			this.buttonUnpatch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonUnpatch.Location = new System.Drawing.Point(14, 42);
			this.buttonUnpatch.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.buttonUnpatch.Name = "buttonUnpatch";
			this.buttonUnpatch.Size = new System.Drawing.Size(595, 47);
			this.buttonUnpatch.TabIndex = 2;
			this.buttonUnpatch.Text = "Remove Patches / Restore Original";
			this.buttonUnpatch.UseVisualStyleBackColor = true;
			this.buttonUnpatch.Click += new System.EventHandler(this.buttonUnpatch_Click);
			// 
			// checkBoxAssetPatches
			// 
			this.checkBoxAssetPatches.AutoSize = true;
			this.checkBoxAssetPatches.Checked = true;
			this.checkBoxAssetPatches.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxAssetPatches.Location = new System.Drawing.Point(113, 114);
			this.checkBoxAssetPatches.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxAssetPatches.Name = "checkBoxAssetPatches";
			this.checkBoxAssetPatches.Size = new System.Drawing.Size(237, 19);
			this.checkBoxAssetPatches.TabIndex = 4;
			this.checkBoxAssetPatches.Text = "Apply fixes for known script/asset errors";
			this.checkBoxAssetPatches.UseVisualStyleBackColor = true;
			// 
			// buttonAssetFixDetails
			// 
			this.buttonAssetFixDetails.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonAssetFixDetails.Location = new System.Drawing.Point(449, 96);
			this.buttonAssetFixDetails.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.buttonAssetFixDetails.Name = "buttonAssetFixDetails";
			this.buttonAssetFixDetails.Size = new System.Drawing.Size(160, 51);
			this.buttonAssetFixDetails.TabIndex = 5;
			this.buttonAssetFixDetails.Text = "Show asset fix details... (may contain spoilers)";
			this.buttonAssetFixDetails.UseVisualStyleBackColor = true;
			this.buttonAssetFixDetails.Click += new System.EventHandler(this.buttonAssetFixDetails_Click);
			// 
			// checkBoxAllowNightmare
			// 
			this.checkBoxAllowNightmare.AutoSize = true;
			this.checkBoxAllowNightmare.Checked = true;
			this.checkBoxAllowNightmare.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxAllowNightmare.Location = new System.Drawing.Point(113, 141);
			this.checkBoxAllowNightmare.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxAllowNightmare.Name = "checkBoxAllowNightmare";
			this.checkBoxAllowNightmare.Size = new System.Drawing.Size(323, 19);
			this.checkBoxAllowNightmare.TabIndex = 6;
			this.checkBoxAllowNightmare.Text = "Allow changing difficulty to Nightmare mid-playthough";
			this.checkBoxAllowNightmare.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisableMouseCam
			// 
			this.checkBoxDisableMouseCam.AutoSize = true;
			this.checkBoxDisableMouseCam.Location = new System.Drawing.Point(113, 194);
			this.checkBoxDisableMouseCam.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxDisableMouseCam.Name = "checkBoxDisableMouseCam";
			this.checkBoxDisableMouseCam.Size = new System.Drawing.Size(254, 19);
			this.checkBoxDisableMouseCam.TabIndex = 9;
			this.checkBoxDisableMouseCam.Text = "Disable Mouse Capture and Mouse Camera";
			this.checkBoxDisableMouseCam.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisablePauseOnFocusLoss
			// 
			this.checkBoxDisablePauseOnFocusLoss.AutoSize = true;
			this.checkBoxDisablePauseOnFocusLoss.Location = new System.Drawing.Point(113, 244);
			this.checkBoxDisablePauseOnFocusLoss.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxDisablePauseOnFocusLoss.Name = "checkBoxDisablePauseOnFocusLoss";
			this.checkBoxDisablePauseOnFocusLoss.Size = new System.Drawing.Size(242, 19);
			this.checkBoxDisablePauseOnFocusLoss.TabIndex = 11;
			this.checkBoxDisablePauseOnFocusLoss.Text = "Keep game running when in Background";
			this.checkBoxDisablePauseOnFocusLoss.UseVisualStyleBackColor = true;
			// 
			// checkBoxButtonLayout
			// 
			this.checkBoxButtonLayout.AutoSize = true;
			this.checkBoxButtonLayout.Location = new System.Drawing.Point(113, 167);
			this.checkBoxButtonLayout.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxButtonLayout.Name = "checkBoxButtonLayout";
			this.checkBoxButtonLayout.Size = new System.Drawing.Size(235, 19);
			this.checkBoxButtonLayout.TabIndex = 7;
			this.checkBoxButtonLayout.Text = "Force Confirm/Cancel button layout to:";
			this.checkBoxButtonLayout.UseVisualStyleBackColor = true;
			this.checkBoxButtonLayout.CheckedChanged += new System.EventHandler(this.checkBoxButtonLayout_CheckedChanged);
			// 
			// comboBoxButtonLayout
			// 
			this.comboBoxButtonLayout.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.comboBoxButtonLayout.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxButtonLayout.FormattingEnabled = true;
			this.comboBoxButtonLayout.Location = new System.Drawing.Point(354, 164);
			this.comboBoxButtonLayout.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.comboBoxButtonLayout.Name = "comboBoxButtonLayout";
			this.comboBoxButtonLayout.Size = new System.Drawing.Size(255, 23);
			this.comboBoxButtonLayout.TabIndex = 8;
			// 
			// checkBoxShowMouseCursor
			// 
			this.checkBoxShowMouseCursor.AutoSize = true;
			this.checkBoxShowMouseCursor.Location = new System.Drawing.Point(113, 219);
			this.checkBoxShowMouseCursor.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxShowMouseCursor.Name = "checkBoxShowMouseCursor";
			this.checkBoxShowMouseCursor.Size = new System.Drawing.Size(132, 19);
			this.checkBoxShowMouseCursor.TabIndex = 10;
			this.checkBoxShowMouseCursor.Text = "Show Mouse Cursor";
			this.checkBoxShowMouseCursor.UseVisualStyleBackColor = true;
			// 
			// Sen4Form
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(623, 337);
			this.Controls.Add(this.checkBoxShowMouseCursor);
			this.Controls.Add(this.comboBoxButtonLayout);
			this.Controls.Add(this.checkBoxButtonLayout);
			this.Controls.Add(this.checkBoxDisablePauseOnFocusLoss);
			this.Controls.Add(this.checkBoxDisableMouseCam);
			this.Controls.Add(this.checkBoxAllowNightmare);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxAssetPatches);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.Name = "Sen4Form";
			this.Text = "Patch Cold Steel 4";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxAssetPatches;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.CheckBox checkBoxAllowNightmare;
		private System.Windows.Forms.CheckBox checkBoxDisableMouseCam;
		private System.Windows.Forms.CheckBox checkBoxDisablePauseOnFocusLoss;
		private System.Windows.Forms.CheckBox checkBoxButtonLayout;
		private System.Windows.Forms.ComboBox comboBoxButtonLayout;
		private System.Windows.Forms.CheckBox checkBoxShowMouseCursor;
	}
}