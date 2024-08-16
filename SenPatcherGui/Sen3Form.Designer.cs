namespace SenPatcherGui {
	partial class Sen3Form {
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
			this.checkBoxFixButtonRemapping = new System.Windows.Forms.CheckBox();
			this.labelFile = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxAssetPatches = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.checkBoxAllowNightmare = new System.Windows.Forms.CheckBox();
			this.checkBoxDisableMouseCam = new System.Windows.Forms.CheckBox();
			this.checkBoxDisablePauseOnFocusLoss = new System.Windows.Forms.CheckBox();
			this.checkBoxControllerMapping = new System.Windows.Forms.CheckBox();
			this.checkBoxShowMouseCursor = new System.Windows.Forms.CheckBox();
			this.checkBoxForceXInput = new System.Windows.Forms.CheckBox();
			this.checkBoxBgmEnqueueingLogic = new System.Windows.Forms.CheckBox();
			this.checkBoxTurboToggle = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
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
			// checkBoxFixButtonRemapping
			// 
			this.checkBoxFixButtonRemapping.AutoSize = true;
			this.checkBoxFixButtonRemapping.Checked = true;
			this.checkBoxFixButtonRemapping.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxFixButtonRemapping.Location = new System.Drawing.Point(97, 145);
			this.checkBoxFixButtonRemapping.Name = "checkBoxFixButtonRemapping";
			this.checkBoxFixButtonRemapping.Size = new System.Drawing.Size(201, 17);
			this.checkBoxFixButtonRemapping.TabIndex = 6;
			this.checkBoxFixButtonRemapping.Text = "Fix in-game button remapping options";
			this.checkBoxFixButtonRemapping.UseVisualStyleBackColor = true;
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
			this.buttonPatch.Location = new System.Drawing.Point(12, 332);
			this.buttonPatch.Name = "buttonPatch";
			this.buttonPatch.Size = new System.Drawing.Size(510, 41);
			this.buttonPatch.TabIndex = 13;
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
			// checkBoxAllowNightmare
			// 
			this.checkBoxAllowNightmare.AutoSize = true;
			this.checkBoxAllowNightmare.Checked = true;
			this.checkBoxAllowNightmare.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxAllowNightmare.Location = new System.Drawing.Point(97, 168);
			this.checkBoxAllowNightmare.Name = "checkBoxAllowNightmare";
			this.checkBoxAllowNightmare.Size = new System.Drawing.Size(276, 17);
			this.checkBoxAllowNightmare.TabIndex = 7;
			this.checkBoxAllowNightmare.Text = "Allow changing difficulty to Nightmare mid-playthough";
			this.checkBoxAllowNightmare.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisableMouseCam
			// 
			this.checkBoxDisableMouseCam.AutoSize = true;
			this.checkBoxDisableMouseCam.Location = new System.Drawing.Point(97, 237);
			this.checkBoxDisableMouseCam.Name = "checkBoxDisableMouseCam";
			this.checkBoxDisableMouseCam.Size = new System.Drawing.Size(231, 17);
			this.checkBoxDisableMouseCam.TabIndex = 9;
			this.checkBoxDisableMouseCam.Text = "Disable Mouse Capture and Mouse Camera";
			this.checkBoxDisableMouseCam.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisablePauseOnFocusLoss
			// 
			this.checkBoxDisablePauseOnFocusLoss.AutoSize = true;
			this.checkBoxDisablePauseOnFocusLoss.Location = new System.Drawing.Point(97, 280);
			this.checkBoxDisablePauseOnFocusLoss.Name = "checkBoxDisablePauseOnFocusLoss";
			this.checkBoxDisablePauseOnFocusLoss.Size = new System.Drawing.Size(219, 17);
			this.checkBoxDisablePauseOnFocusLoss.TabIndex = 11;
			this.checkBoxDisablePauseOnFocusLoss.Text = "Keep game running when in Background";
			this.checkBoxDisablePauseOnFocusLoss.UseVisualStyleBackColor = true;
			// 
			// checkBoxControllerMapping
			// 
			this.checkBoxControllerMapping.AutoSize = true;
			this.checkBoxControllerMapping.Checked = true;
			this.checkBoxControllerMapping.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxControllerMapping.Location = new System.Drawing.Point(97, 191);
			this.checkBoxControllerMapping.Name = "checkBoxControllerMapping";
			this.checkBoxControllerMapping.Size = new System.Drawing.Size(409, 17);
			this.checkBoxControllerMapping.TabIndex = 8;
			this.checkBoxControllerMapping.Text = "Fix inconsistent Controller Button Prompts and Mappings when remapping buttons";
			this.checkBoxControllerMapping.UseVisualStyleBackColor = true;
			// 
			// checkBoxShowMouseCursor
			// 
			this.checkBoxShowMouseCursor.AutoSize = true;
			this.checkBoxShowMouseCursor.Location = new System.Drawing.Point(97, 258);
			this.checkBoxShowMouseCursor.Name = "checkBoxShowMouseCursor";
			this.checkBoxShowMouseCursor.Size = new System.Drawing.Size(121, 17);
			this.checkBoxShowMouseCursor.TabIndex = 10;
			this.checkBoxShowMouseCursor.Text = "Show Mouse Cursor";
			this.checkBoxShowMouseCursor.UseVisualStyleBackColor = true;
			// 
			// checkBoxForceXInput
			// 
			this.checkBoxForceXInput.AutoSize = true;
			this.checkBoxForceXInput.Location = new System.Drawing.Point(97, 302);
			this.checkBoxForceXInput.Name = "checkBoxForceXInput";
			this.checkBoxForceXInput.Size = new System.Drawing.Size(370, 17);
			this.checkBoxForceXInput.TabIndex = 12;
			this.checkBoxForceXInput.Text = "Force all controllers as XInput (may fix issues with newer Xbox controllers)";
			this.checkBoxForceXInput.UseVisualStyleBackColor = true;
			// 
			// checkBoxBgmEnqueueingLogic
			// 
			this.checkBoxBgmEnqueueingLogic.AutoSize = true;
			this.checkBoxBgmEnqueueingLogic.Checked = true;
			this.checkBoxBgmEnqueueingLogic.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxBgmEnqueueingLogic.Location = new System.Drawing.Point(97, 214);
			this.checkBoxBgmEnqueueingLogic.Name = "checkBoxBgmEnqueueingLogic";
			this.checkBoxBgmEnqueueingLogic.Size = new System.Drawing.Size(323, 17);
			this.checkBoxBgmEnqueueingLogic.TabIndex = 21;
			this.checkBoxBgmEnqueueingLogic.Text = "Fix wrong BGM when BGM is changed while track is fading out";
			this.checkBoxBgmEnqueueingLogic.UseVisualStyleBackColor = true;
			// 
			// checkBoxTurboToggle
			// 
			this.checkBoxTurboToggle.AutoSize = true;
			this.checkBoxTurboToggle.Location = new System.Drawing.Point(97, 122);
			this.checkBoxTurboToggle.Name = "checkBoxTurboToggle";
			this.checkBoxTurboToggle.Size = new System.Drawing.Size(222, 17);
			this.checkBoxTurboToggle.TabIndex = 22;
			this.checkBoxTurboToggle.Text = "Make turbo mode a toggle instead of hold";
			this.checkBoxTurboToggle.UseVisualStyleBackColor = true;
			// 
			// Sen3Form
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(534, 385);
			this.Controls.Add(this.checkBoxTurboToggle);
			this.Controls.Add(this.checkBoxBgmEnqueueingLogic);
			this.Controls.Add(this.checkBoxForceXInput);
			this.Controls.Add(this.checkBoxShowMouseCursor);
			this.Controls.Add(this.checkBoxControllerMapping);
			this.Controls.Add(this.checkBoxDisablePauseOnFocusLoss);
			this.Controls.Add(this.checkBoxDisableMouseCam);
			this.Controls.Add(this.checkBoxAllowNightmare);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxAssetPatches);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.checkBoxFixButtonRemapping);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Name = "Sen3Form";
			this.Text = "Patch Cold Steel 3";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox checkBoxFixButtonRemapping;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxAssetPatches;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.CheckBox checkBoxAllowNightmare;
		private System.Windows.Forms.CheckBox checkBoxDisableMouseCam;
		private System.Windows.Forms.CheckBox checkBoxDisablePauseOnFocusLoss;
		private System.Windows.Forms.CheckBox checkBoxControllerMapping;
		private System.Windows.Forms.CheckBox checkBoxShowMouseCursor;
		private System.Windows.Forms.CheckBox checkBoxForceXInput;
		private System.Windows.Forms.CheckBox checkBoxBgmEnqueueingLogic;
		private System.Windows.Forms.CheckBox checkBoxTurboToggle;
	}
}