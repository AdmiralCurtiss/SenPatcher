namespace SenPatcherGui {
	partial class Sen1Form {
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Sen1Form));
			this.label1 = new System.Windows.Forms.Label();
			this.labelFile = new System.Windows.Forms.Label();
			this.checkBoxBattleAutoSkip = new System.Windows.Forms.CheckBox();
			this.label2 = new System.Windows.Forms.Label();
			this.checkBoxAllowR2InTurboMode = new System.Windows.Forms.CheckBox();
			this.label4 = new System.Windows.Forms.Label();
			this.comboBoxTurboModeKey = new System.Windows.Forms.ComboBox();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.label5 = new System.Windows.Forms.Label();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxAssetPatches = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.checkBoxFixVoiceFileLang = new System.Windows.Forms.CheckBox();
			this.checkBoxDisableMouseCam = new System.Windows.Forms.CheckBox();
			this.checkBoxDisablePauseOnFocusLoss = new System.Windows.Forms.CheckBox();
			this.checkBoxArtsSupport = new System.Windows.Forms.CheckBox();
			this.checkBoxForce0Kerning = new System.Windows.Forms.CheckBox();
			this.checkBoxShowMouseCursor = new System.Windows.Forms.CheckBox();
			this.checkBoxForceXInput = new System.Windows.Forms.CheckBox();
			this.checkBoxBgmEnqueueingLogic = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
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
			// labelFile
			// 
			this.labelFile.AutoSize = true;
			this.labelFile.Location = new System.Drawing.Point(57, 13);
			this.labelFile.Name = "labelFile";
			this.labelFile.Size = new System.Drawing.Size(16, 13);
			this.labelFile.TabIndex = 1;
			this.labelFile.Text = "---";
			// 
			// checkBoxBattleAutoSkip
			// 
			this.checkBoxBattleAutoSkip.AutoSize = true;
			this.checkBoxBattleAutoSkip.Checked = true;
			this.checkBoxBattleAutoSkip.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxBattleAutoSkip.Location = new System.Drawing.Point(97, 122);
			this.checkBoxBattleAutoSkip.Name = "checkBoxBattleAutoSkip";
			this.checkBoxBattleAutoSkip.Size = new System.Drawing.Size(207, 17);
			this.checkBoxBattleAutoSkip.TabIndex = 6;
			this.checkBoxBattleAutoSkip.Text = "Remove animation skip in Turbo mode";
			this.checkBoxBattleAutoSkip.UseVisualStyleBackColor = true;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(12, 99);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(79, 13);
			this.label2.TabIndex = 3;
			this.label2.Text = "Patch Settings:";
			// 
			// checkBoxAllowR2InTurboMode
			// 
			this.checkBoxAllowR2InTurboMode.AutoSize = true;
			this.checkBoxAllowR2InTurboMode.Location = new System.Drawing.Point(97, 323);
			this.checkBoxAllowR2InTurboMode.Name = "checkBoxAllowR2InTurboMode";
			this.checkBoxAllowR2InTurboMode.Size = new System.Drawing.Size(280, 17);
			this.checkBoxAllowR2InTurboMode.TabIndex = 15;
			this.checkBoxAllowR2InTurboMode.Text = "Enable R2 Notebook Shortcut when Turbo is enabled";
			this.checkBoxAllowR2InTurboMode.UseVisualStyleBackColor = true;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(94, 349);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(87, 13);
			this.label4.TabIndex = 16;
			this.label4.Text = "Turbo mode key:";
			// 
			// comboBoxTurboModeKey
			// 
			this.comboBoxTurboModeKey.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxTurboModeKey.FormattingEnabled = true;
			this.comboBoxTurboModeKey.Location = new System.Drawing.Point(187, 346);
			this.comboBoxTurboModeKey.Name = "comboBoxTurboModeKey";
			this.comboBoxTurboModeKey.Size = new System.Drawing.Size(126, 21);
			this.comboBoxTurboModeKey.TabIndex = 17;
			// 
			// buttonPatch
			// 
			this.buttonPatch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonPatch.Location = new System.Drawing.Point(13, 439);
			this.buttonPatch.Name = "buttonPatch";
			this.buttonPatch.Size = new System.Drawing.Size(510, 41);
			this.buttonPatch.TabIndex = 19;
			this.buttonPatch.Text = "Patch!";
			this.buttonPatch.UseVisualStyleBackColor = true;
			this.buttonPatch.Click += new System.EventHandler(this.buttonPatch_Click);
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(94, 376);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(408, 52);
			this.label5.TabIndex = 18;
			this.label5.Text = resources.GetString("label5.Text");
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
			// checkBoxAssetPatches
			// 
			this.checkBoxAssetPatches.AutoSize = true;
			this.checkBoxAssetPatches.Checked = true;
			this.checkBoxAssetPatches.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxAssetPatches.Location = new System.Drawing.Point(97, 99);
			this.checkBoxAssetPatches.Name = "checkBoxAssetPatches";
			this.checkBoxAssetPatches.Size = new System.Drawing.Size(213, 17);
			this.checkBoxAssetPatches.TabIndex = 4;
			this.checkBoxAssetPatches.Text = "Apply fixes for known script/asset errors";
			this.checkBoxAssetPatches.UseVisualStyleBackColor = true;
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
			// checkBoxFixVoiceFileLang
			// 
			this.checkBoxFixVoiceFileLang.AutoSize = true;
			this.checkBoxFixVoiceFileLang.Checked = true;
			this.checkBoxFixVoiceFileLang.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxFixVoiceFileLang.Location = new System.Drawing.Point(97, 145);
			this.checkBoxFixVoiceFileLang.Name = "checkBoxFixVoiceFileLang";
			this.checkBoxFixVoiceFileLang.Size = new System.Drawing.Size(369, 17);
			this.checkBoxFixVoiceFileLang.TabIndex = 8;
			this.checkBoxFixVoiceFileLang.Text = "Load voice metadata matching voice language rather than text language";
			this.checkBoxFixVoiceFileLang.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisableMouseCam
			// 
			this.checkBoxDisableMouseCam.AutoSize = true;
			this.checkBoxDisableMouseCam.Location = new System.Drawing.Point(97, 237);
			this.checkBoxDisableMouseCam.Name = "checkBoxDisableMouseCam";
			this.checkBoxDisableMouseCam.Size = new System.Drawing.Size(231, 17);
			this.checkBoxDisableMouseCam.TabIndex = 11;
			this.checkBoxDisableMouseCam.Text = "Disable Mouse Capture and Mouse Camera";
			this.checkBoxDisableMouseCam.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisablePauseOnFocusLoss
			// 
			this.checkBoxDisablePauseOnFocusLoss.AutoSize = true;
			this.checkBoxDisablePauseOnFocusLoss.Location = new System.Drawing.Point(97, 280);
			this.checkBoxDisablePauseOnFocusLoss.Name = "checkBoxDisablePauseOnFocusLoss";
			this.checkBoxDisablePauseOnFocusLoss.Size = new System.Drawing.Size(219, 17);
			this.checkBoxDisablePauseOnFocusLoss.TabIndex = 13;
			this.checkBoxDisablePauseOnFocusLoss.Text = "Keep game running when in Background";
			this.checkBoxDisablePauseOnFocusLoss.UseVisualStyleBackColor = true;
			// 
			// checkBoxArtsSupport
			// 
			this.checkBoxArtsSupport.AutoSize = true;
			this.checkBoxArtsSupport.Checked = true;
			this.checkBoxArtsSupport.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxArtsSupport.Location = new System.Drawing.Point(97, 168);
			this.checkBoxArtsSupport.Name = "checkBoxArtsSupport";
			this.checkBoxArtsSupport.Size = new System.Drawing.Size(308, 17);
			this.checkBoxArtsSupport.TabIndex = 9;
			this.checkBoxArtsSupport.Text = "Fix Arts Support cut-in issues when not running at 1280x720";
			this.checkBoxArtsSupport.UseVisualStyleBackColor = true;
			// 
			// checkBoxForce0Kerning
			// 
			this.checkBoxForce0Kerning.AutoSize = true;
			this.checkBoxForce0Kerning.Location = new System.Drawing.Point(97, 214);
			this.checkBoxForce0Kerning.Name = "checkBoxForce0Kerning";
			this.checkBoxForce0Kerning.Size = new System.Drawing.Size(396, 17);
			this.checkBoxForce0Kerning.TabIndex = 10;
			this.checkBoxForce0Kerning.Text = "Adjust font spacing for use with HD Cuprum font from CS3 or HD Texture Pack";
			this.checkBoxForce0Kerning.UseVisualStyleBackColor = true;
			// 
			// checkBoxShowMouseCursor
			// 
			this.checkBoxShowMouseCursor.AutoSize = true;
			this.checkBoxShowMouseCursor.Location = new System.Drawing.Point(97, 258);
			this.checkBoxShowMouseCursor.Name = "checkBoxShowMouseCursor";
			this.checkBoxShowMouseCursor.Size = new System.Drawing.Size(121, 17);
			this.checkBoxShowMouseCursor.TabIndex = 12;
			this.checkBoxShowMouseCursor.Text = "Show Mouse Cursor";
			this.checkBoxShowMouseCursor.UseVisualStyleBackColor = true;
			// 
			// checkBoxForceXInput
			// 
			this.checkBoxForceXInput.AutoSize = true;
			this.checkBoxForceXInput.Location = new System.Drawing.Point(97, 302);
			this.checkBoxForceXInput.Name = "checkBoxForceXInput";
			this.checkBoxForceXInput.Size = new System.Drawing.Size(370, 17);
			this.checkBoxForceXInput.TabIndex = 14;
			this.checkBoxForceXInput.Text = "Force all controllers as XInput (may fix issues with newer Xbox controllers)";
			this.checkBoxForceXInput.UseVisualStyleBackColor = true;
			// 
			// checkBoxBgmEnqueueingLogic
			// 
			this.checkBoxBgmEnqueueingLogic.AutoSize = true;
			this.checkBoxBgmEnqueueingLogic.Checked = true;
			this.checkBoxBgmEnqueueingLogic.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxBgmEnqueueingLogic.Location = new System.Drawing.Point(97, 191);
			this.checkBoxBgmEnqueueingLogic.Name = "checkBoxBgmEnqueueingLogic";
			this.checkBoxBgmEnqueueingLogic.Size = new System.Drawing.Size(323, 17);
			this.checkBoxBgmEnqueueingLogic.TabIndex = 20;
			this.checkBoxBgmEnqueueingLogic.Text = "Fix wrong BGM when BGM is changed while track is fading out";
			this.checkBoxBgmEnqueueingLogic.UseVisualStyleBackColor = true;
			// 
			// Sen1Form
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(534, 490);
			this.Controls.Add(this.checkBoxBgmEnqueueingLogic);
			this.Controls.Add(this.checkBoxForceXInput);
			this.Controls.Add(this.checkBoxShowMouseCursor);
			this.Controls.Add(this.checkBoxForce0Kerning);
			this.Controls.Add(this.checkBoxArtsSupport);
			this.Controls.Add(this.checkBoxDisablePauseOnFocusLoss);
			this.Controls.Add(this.checkBoxDisableMouseCam);
			this.Controls.Add(this.checkBoxFixVoiceFileLang);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxAssetPatches);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.comboBoxTurboModeKey);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.checkBoxAllowR2InTurboMode);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.checkBoxBattleAutoSkip);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Name = "Sen1Form";
			this.Text = "Patch Cold Steel 1";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.CheckBox checkBoxBattleAutoSkip;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox checkBoxAllowR2InTurboMode;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.ComboBox comboBoxTurboModeKey;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxAssetPatches;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.CheckBox checkBoxFixVoiceFileLang;
		private System.Windows.Forms.CheckBox checkBoxDisableMouseCam;
		private System.Windows.Forms.CheckBox checkBoxDisablePauseOnFocusLoss;
		private System.Windows.Forms.CheckBox checkBoxArtsSupport;
		private System.Windows.Forms.CheckBox checkBoxForce0Kerning;
		private System.Windows.Forms.CheckBox checkBoxShowMouseCursor;
		private System.Windows.Forms.CheckBox checkBoxForceXInput;
		private System.Windows.Forms.CheckBox checkBoxBgmEnqueueingLogic;
	}
}