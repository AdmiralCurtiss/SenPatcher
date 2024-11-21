namespace SenPatcherGui {
	partial class TXForm {
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
			this.labelPatchHeader = new System.Windows.Forms.Label();
			this.checkBoxSkipMovies = new System.Windows.Forms.CheckBox();
			this.labelFile = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxSkipLogos = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.comboBoxGameLanguage = new System.Windows.Forms.ComboBox();
			this.label4 = new System.Windows.Forms.Label();
			this.numericUpDownTurboFactor = new System.Windows.Forms.NumericUpDown();
			this.labelTurboFactor = new System.Windows.Forms.Label();
			this.checkBoxTurboToggle = new System.Windows.Forms.CheckBox();
			this.checkBoxShowMouseCursor = new System.Windows.Forms.CheckBox();
			this.checkBoxEnableBackgroundControllerInput = new System.Windows.Forms.CheckBox();
			this.checkBoxDisableMouseCam = new System.Windows.Forms.CheckBox();
			this.labelTurboDescription = new System.Windows.Forms.Label();
			this.checkBoxAssetPatches = new System.Windows.Forms.CheckBox();
			this.labelTurboHeader = new System.Windows.Forms.Label();
			this.checkBoxDisableCamAutoCenter = new System.Windows.Forms.CheckBox();
			this.checkBoxFixBgmResume = new System.Windows.Forms.CheckBox();
			((System.ComponentModel.ISupportInitialize)(this.numericUpDownTurboFactor)).BeginInit();
			this.SuspendLayout();
			// 
			// labelPatchHeader
			// 
			this.labelPatchHeader.AutoSize = true;
			this.labelPatchHeader.Location = new System.Drawing.Point(12, 99);
			this.labelPatchHeader.Name = "labelPatchHeader";
			this.labelPatchHeader.Size = new System.Drawing.Size(79, 13);
			this.labelPatchHeader.TabIndex = 3;
			this.labelPatchHeader.Text = "Patch Settings:";
			// 
			// checkBoxSkipMovies
			// 
			this.checkBoxSkipMovies.AutoSize = true;
			this.checkBoxSkipMovies.Location = new System.Drawing.Point(98, 226);
			this.checkBoxSkipMovies.Name = "checkBoxSkipMovies";
			this.checkBoxSkipMovies.Size = new System.Drawing.Size(96, 17);
			this.checkBoxSkipMovies.TabIndex = 12;
			this.checkBoxSkipMovies.Text = "Skip all movies";
			this.checkBoxSkipMovies.UseVisualStyleBackColor = true;
			// 
			// labelFile
			// 
			this.labelFile.AutoSize = true;
			this.labelFile.Location = new System.Drawing.Point(57, 13);
			this.labelFile.Name = "labelFile";
			this.labelFile.Size = new System.Drawing.Size(16, 13);
			this.labelFile.TabIndex = 1;
			this.labelFile.Text = "---";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(35, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "Path: ";
			// 
			// buttonPatch
			// 
			this.buttonPatch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonPatch.Location = new System.Drawing.Point(12, 398);
			this.buttonPatch.Name = "buttonPatch";
			this.buttonPatch.Size = new System.Drawing.Size(510, 41);
			this.buttonPatch.TabIndex = 20;
			this.buttonPatch.Text = "Patch!";
			this.buttonPatch.UseVisualStyleBackColor = true;
			this.buttonPatch.Click += new System.EventHandler(this.buttonPatch_Click);
			// 
			// buttonUnpatch
			// 
			this.buttonUnpatch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonUnpatch.Location = new System.Drawing.Point(12, 36);
			this.buttonUnpatch.Name = "buttonUnpatch";
			this.buttonUnpatch.Size = new System.Drawing.Size(510, 41);
			this.buttonUnpatch.TabIndex = 2;
			this.buttonUnpatch.Text = "Remove Patches / Restore Original";
			this.buttonUnpatch.UseVisualStyleBackColor = true;
			this.buttonUnpatch.Click += new System.EventHandler(this.buttonUnpatch_Click);
			// 
			// checkBoxSkipLogos
			// 
			this.checkBoxSkipLogos.AutoSize = true;
			this.checkBoxSkipLogos.Checked = true;
			this.checkBoxSkipLogos.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxSkipLogos.Location = new System.Drawing.Point(98, 203);
			this.checkBoxSkipLogos.Name = "checkBoxSkipLogos";
			this.checkBoxSkipLogos.Size = new System.Drawing.Size(110, 17);
			this.checkBoxSkipLogos.TabIndex = 11;
			this.checkBoxSkipLogos.Text = "Skip startup logos";
			this.checkBoxSkipLogos.UseVisualStyleBackColor = true;
			// 
			// buttonAssetFixDetails
			// 
			this.buttonAssetFixDetails.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonAssetFixDetails.Location = new System.Drawing.Point(385, 83);
			this.buttonAssetFixDetails.Name = "buttonAssetFixDetails";
			this.buttonAssetFixDetails.Size = new System.Drawing.Size(137, 44);
			this.buttonAssetFixDetails.TabIndex = 5;
			this.buttonAssetFixDetails.Text = "Show asset fix details... (may contain spoilers)";
			this.buttonAssetFixDetails.UseVisualStyleBackColor = true;
			this.buttonAssetFixDetails.Click += new System.EventHandler(this.buttonAssetFixDetails_Click);
			// 
			// comboBoxGameLanguage
			// 
			this.comboBoxGameLanguage.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxGameLanguage.FormattingEnabled = true;
			this.comboBoxGameLanguage.Location = new System.Drawing.Point(186, 362);
			this.comboBoxGameLanguage.Name = "comboBoxGameLanguage";
			this.comboBoxGameLanguage.Size = new System.Drawing.Size(126, 21);
			this.comboBoxGameLanguage.TabIndex = 19;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(95, 365);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(85, 13);
			this.label4.TabIndex = 18;
			this.label4.Text = "Game language:";
			// 
			// numericUpDownTurboFactor
			// 
			this.numericUpDownTurboFactor.Location = new System.Drawing.Point(286, 121);
			this.numericUpDownTurboFactor.Maximum = new decimal(new int[] {
            6,
            0,
            0,
            0});
			this.numericUpDownTurboFactor.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
			this.numericUpDownTurboFactor.Name = "numericUpDownTurboFactor";
			this.numericUpDownTurboFactor.Size = new System.Drawing.Size(33, 20);
			this.numericUpDownTurboFactor.TabIndex = 8;
			this.numericUpDownTurboFactor.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
			// 
			// labelTurboFactor
			// 
			this.labelTurboFactor.AutoSize = true;
			this.labelTurboFactor.Location = new System.Drawing.Point(209, 123);
			this.labelTurboFactor.Name = "labelTurboFactor";
			this.labelTurboFactor.Size = new System.Drawing.Size(71, 13);
			this.labelTurboFactor.TabIndex = 7;
			this.labelTurboFactor.Text = "Turbo Factor:";
			// 
			// checkBoxTurboToggle
			// 
			this.checkBoxTurboToggle.AutoSize = true;
			this.checkBoxTurboToggle.Location = new System.Drawing.Point(212, 147);
			this.checkBoxTurboToggle.Name = "checkBoxTurboToggle";
			this.checkBoxTurboToggle.Size = new System.Drawing.Size(222, 17);
			this.checkBoxTurboToggle.TabIndex = 9;
			this.checkBoxTurboToggle.Text = "Make turbo mode a toggle instead of hold";
			this.checkBoxTurboToggle.UseVisualStyleBackColor = true;
			// 
			// checkBoxShowMouseCursor
			// 
			this.checkBoxShowMouseCursor.AutoSize = true;
			this.checkBoxShowMouseCursor.Location = new System.Drawing.Point(98, 317);
			this.checkBoxShowMouseCursor.Name = "checkBoxShowMouseCursor";
			this.checkBoxShowMouseCursor.Size = new System.Drawing.Size(121, 17);
			this.checkBoxShowMouseCursor.TabIndex = 16;
			this.checkBoxShowMouseCursor.Text = "Show Mouse Cursor";
			this.checkBoxShowMouseCursor.UseVisualStyleBackColor = true;
			// 
			// checkBoxEnableBackgroundControllerInput
			// 
			this.checkBoxEnableBackgroundControllerInput.AutoSize = true;
			this.checkBoxEnableBackgroundControllerInput.Location = new System.Drawing.Point(98, 339);
			this.checkBoxEnableBackgroundControllerInput.Name = "checkBoxEnableBackgroundControllerInput";
			this.checkBoxEnableBackgroundControllerInput.Size = new System.Drawing.Size(278, 17);
			this.checkBoxEnableBackgroundControllerInput.TabIndex = 17;
			this.checkBoxEnableBackgroundControllerInput.Text = "Keep processing controller input when in Background";
			this.checkBoxEnableBackgroundControllerInput.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisableMouseCam
			// 
			this.checkBoxDisableMouseCam.AutoSize = true;
			this.checkBoxDisableMouseCam.Location = new System.Drawing.Point(98, 295);
			this.checkBoxDisableMouseCam.Name = "checkBoxDisableMouseCam";
			this.checkBoxDisableMouseCam.Size = new System.Drawing.Size(135, 17);
			this.checkBoxDisableMouseCam.TabIndex = 15;
			this.checkBoxDisableMouseCam.Text = "Disable Mouse Camera";
			this.checkBoxDisableMouseCam.UseVisualStyleBackColor = true;
			// 
			// labelTurboDescription
			// 
			this.labelTurboDescription.AutoSize = true;
			this.labelTurboDescription.Location = new System.Drawing.Point(94, 167);
			this.labelTurboDescription.Name = "labelTurboDescription";
			this.labelTurboDescription.Size = new System.Drawing.Size(381, 26);
			this.labelTurboDescription.TabIndex = 10;
			this.labelTurboDescription.Text = "The default button for Turbo Mode is L3 on controller and Left Ctrl on keyboard.\r" +
    "\nThis can be changed in the in-game Button Configuration options.";
			// 
			// checkBoxAssetPatches
			// 
			this.checkBoxAssetPatches.AutoSize = true;
			this.checkBoxAssetPatches.Checked = true;
			this.checkBoxAssetPatches.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxAssetPatches.Location = new System.Drawing.Point(98, 99);
			this.checkBoxAssetPatches.Name = "checkBoxAssetPatches";
			this.checkBoxAssetPatches.Size = new System.Drawing.Size(213, 17);
			this.checkBoxAssetPatches.TabIndex = 4;
			this.checkBoxAssetPatches.Text = "Apply fixes for known script/asset errors";
			this.checkBoxAssetPatches.UseVisualStyleBackColor = true;
			// 
			// labelTurboHeader
			// 
			this.labelTurboHeader.AutoSize = true;
			this.labelTurboHeader.Location = new System.Drawing.Point(94, 123);
			this.labelTurboHeader.Name = "labelTurboHeader";
			this.labelTurboHeader.Size = new System.Drawing.Size(109, 13);
			this.labelTurboHeader.TabIndex = 6;
			this.labelTurboHeader.Text = "Turbo Mode Settings:";
			// 
			// checkBoxDisableCamAutoCenter
			// 
			this.checkBoxDisableCamAutoCenter.AutoSize = true;
			this.checkBoxDisableCamAutoCenter.Location = new System.Drawing.Point(98, 272);
			this.checkBoxDisableCamAutoCenter.Name = "checkBoxDisableCamAutoCenter";
			this.checkBoxDisableCamAutoCenter.Size = new System.Drawing.Size(170, 17);
			this.checkBoxDisableCamAutoCenter.TabIndex = 14;
			this.checkBoxDisableCamAutoCenter.Text = "Disable camera auto-centering";
			this.checkBoxDisableCamAutoCenter.UseVisualStyleBackColor = true;
			// 
			// checkBoxFixBgmResume
			// 
			this.checkBoxFixBgmResume.AutoSize = true;
			this.checkBoxFixBgmResume.Checked = true;
			this.checkBoxFixBgmResume.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxFixBgmResume.Location = new System.Drawing.Point(98, 249);
			this.checkBoxFixBgmResume.Name = "checkBoxFixBgmResume";
			this.checkBoxFixBgmResume.Size = new System.Drawing.Size(135, 17);
			this.checkBoxFixBgmResume.TabIndex = 13;
			this.checkBoxFixBgmResume.Text = "Fix music resuming bug";
			this.checkBoxFixBgmResume.UseVisualStyleBackColor = true;
			// 
			// TXForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(534, 451);
			this.Controls.Add(this.checkBoxFixBgmResume);
			this.Controls.Add(this.checkBoxDisableCamAutoCenter);
			this.Controls.Add(this.labelTurboHeader);
			this.Controls.Add(this.checkBoxAssetPatches);
			this.Controls.Add(this.labelTurboDescription);
			this.Controls.Add(this.checkBoxShowMouseCursor);
			this.Controls.Add(this.checkBoxEnableBackgroundControllerInput);
			this.Controls.Add(this.checkBoxDisableMouseCam);
			this.Controls.Add(this.checkBoxTurboToggle);
			this.Controls.Add(this.labelTurboFactor);
			this.Controls.Add(this.numericUpDownTurboFactor);
			this.Controls.Add(this.comboBoxGameLanguage);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxSkipLogos);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.labelPatchHeader);
			this.Controls.Add(this.checkBoxSkipMovies);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Name = "TXForm";
			this.Text = "Patch Tokyo Xanadu eX+";
			((System.ComponentModel.ISupportInitialize)(this.numericUpDownTurboFactor)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Label labelPatchHeader;
		private System.Windows.Forms.CheckBox checkBoxSkipMovies;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxSkipLogos;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.ComboBox comboBoxGameLanguage;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.NumericUpDown numericUpDownTurboFactor;
		private System.Windows.Forms.Label labelTurboFactor;
		private System.Windows.Forms.CheckBox checkBoxTurboToggle;
		private System.Windows.Forms.CheckBox checkBoxShowMouseCursor;
		private System.Windows.Forms.CheckBox checkBoxEnableBackgroundControllerInput;
		private System.Windows.Forms.CheckBox checkBoxDisableMouseCam;
		private System.Windows.Forms.Label labelTurboDescription;
		private System.Windows.Forms.CheckBox checkBoxAssetPatches;
		private System.Windows.Forms.Label labelTurboHeader;
		private System.Windows.Forms.CheckBox checkBoxDisableCamAutoCenter;
		private System.Windows.Forms.CheckBox checkBoxFixBgmResume;
	}
}