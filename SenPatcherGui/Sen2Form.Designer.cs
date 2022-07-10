namespace SenPatcherGui {
	partial class Sen2Form {
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
			this.label5 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.checkBoxBattleAutoSkip = new System.Windows.Forms.CheckBox();
			this.labelFile = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.checkBoxPatchAudioThread = new System.Windows.Forms.CheckBox();
			this.checkBoxBgmEnqueueingLogic = new System.Windows.Forms.CheckBox();
			this.label6 = new System.Windows.Forms.Label();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxAssetPatches = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.checkBoxFixVoiceFileLang = new System.Windows.Forms.CheckBox();
			this.checkBoxDisableMouseCam = new System.Windows.Forms.CheckBox();
			this.checkBoxDisablePauseOnFocusLoss = new System.Windows.Forms.CheckBox();
			this.checkBoxControllerMapping = new System.Windows.Forms.CheckBox();
			this.checkBoxArtsSupport = new System.Windows.Forms.CheckBox();
			this.checkBoxForce0Kerning = new System.Windows.Forms.CheckBox();
			this.checkBoxShowMouseCursor = new System.Windows.Forms.CheckBox();
			this.checkBoxFixBattleScopeCrash = new System.Windows.Forms.CheckBox();
			this.checkBoxForceXInput = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(110, 282);
			this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(452, 30);
			this.label5.TabIndex = 11;
			this.label5.Text = "Makes the audio thread\'s timing more consistent, which fixes overly long BGM fade" +
    "s\r\nand some missing sound effects such as Fie\'s Sweep Craft.\r\n";
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
			// checkBoxBattleAutoSkip
			// 
			this.checkBoxBattleAutoSkip.AutoSize = true;
			this.checkBoxBattleAutoSkip.Checked = true;
			this.checkBoxBattleAutoSkip.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxBattleAutoSkip.Location = new System.Drawing.Point(113, 141);
			this.checkBoxBattleAutoSkip.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxBattleAutoSkip.Name = "checkBoxBattleAutoSkip";
			this.checkBoxBattleAutoSkip.Size = new System.Drawing.Size(231, 19);
			this.checkBoxBattleAutoSkip.TabIndex = 6;
			this.checkBoxBattleAutoSkip.Text = "Remove animation skip in Turbo mode";
			this.checkBoxBattleAutoSkip.UseVisualStyleBackColor = true;
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
			this.buttonPatch.Location = new System.Drawing.Point(14, 532);
			this.buttonPatch.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.buttonPatch.Name = "buttonPatch";
			this.buttonPatch.Size = new System.Drawing.Size(595, 47);
			this.buttonPatch.TabIndex = 20;
			this.buttonPatch.Text = "Patch!";
			this.buttonPatch.UseVisualStyleBackColor = true;
			this.buttonPatch.Click += new System.EventHandler(this.buttonPatch_Click);
			// 
			// checkBoxPatchAudioThread
			// 
			this.checkBoxPatchAudioThread.AutoSize = true;
			this.checkBoxPatchAudioThread.Checked = true;
			this.checkBoxPatchAudioThread.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxPatchAudioThread.Location = new System.Drawing.Point(113, 258);
			this.checkBoxPatchAudioThread.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxPatchAudioThread.Name = "checkBoxPatchAudioThread";
			this.checkBoxPatchAudioThread.Size = new System.Drawing.Size(273, 19);
			this.checkBoxPatchAudioThread.TabIndex = 10;
			this.checkBoxPatchAudioThread.Text = "Replace Audio Timing Thread (Recommended)";
			this.checkBoxPatchAudioThread.UseVisualStyleBackColor = true;
			// 
			// checkBoxBgmEnqueueingLogic
			// 
			this.checkBoxBgmEnqueueingLogic.AutoSize = true;
			this.checkBoxBgmEnqueueingLogic.Checked = true;
			this.checkBoxBgmEnqueueingLogic.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxBgmEnqueueingLogic.Location = new System.Drawing.Point(113, 194);
			this.checkBoxBgmEnqueueingLogic.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxBgmEnqueueingLogic.Name = "checkBoxBgmEnqueueingLogic";
			this.checkBoxBgmEnqueueingLogic.Size = new System.Drawing.Size(263, 19);
			this.checkBoxBgmEnqueueingLogic.TabIndex = 8;
			this.checkBoxBgmEnqueueingLogic.Text = "Adjust BGM queueing logic (Recommended)";
			this.checkBoxBgmEnqueueingLogic.UseVisualStyleBackColor = true;
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(110, 217);
			this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(473, 30);
			this.label6.TabIndex = 9;
			this.label6.Text = "Fixes the missing BGM bug in the intermission as well as the incorrect BGM being " +
    "played\r\nwhen quickly and repeatedly moving between areas with different music.";
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
			// checkBoxFixVoiceFileLang
			// 
			this.checkBoxFixVoiceFileLang.AutoSize = true;
			this.checkBoxFixVoiceFileLang.Checked = true;
			this.checkBoxFixVoiceFileLang.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxFixVoiceFileLang.Location = new System.Drawing.Point(113, 167);
			this.checkBoxFixVoiceFileLang.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxFixVoiceFileLang.Name = "checkBoxFixVoiceFileLang";
			this.checkBoxFixVoiceFileLang.Size = new System.Drawing.Size(409, 19);
			this.checkBoxFixVoiceFileLang.TabIndex = 7;
			this.checkBoxFixVoiceFileLang.Text = "Load voice metadata matching voice language rather than text language";
			this.checkBoxFixVoiceFileLang.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisableMouseCam
			// 
			this.checkBoxDisableMouseCam.AutoSize = true;
			this.checkBoxDisableMouseCam.Location = new System.Drawing.Point(113, 403);
			this.checkBoxDisableMouseCam.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxDisableMouseCam.Name = "checkBoxDisableMouseCam";
			this.checkBoxDisableMouseCam.Size = new System.Drawing.Size(254, 19);
			this.checkBoxDisableMouseCam.TabIndex = 15;
			this.checkBoxDisableMouseCam.Text = "Disable Mouse Capture and Mouse Camera";
			this.checkBoxDisableMouseCam.UseVisualStyleBackColor = true;
			// 
			// checkBoxDisablePauseOnFocusLoss
			// 
			this.checkBoxDisablePauseOnFocusLoss.AutoSize = true;
			this.checkBoxDisablePauseOnFocusLoss.Location = new System.Drawing.Point(113, 453);
			this.checkBoxDisablePauseOnFocusLoss.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxDisablePauseOnFocusLoss.Name = "checkBoxDisablePauseOnFocusLoss";
			this.checkBoxDisablePauseOnFocusLoss.Size = new System.Drawing.Size(242, 19);
			this.checkBoxDisablePauseOnFocusLoss.TabIndex = 17;
			this.checkBoxDisablePauseOnFocusLoss.Text = "Keep game running when in Background";
			this.checkBoxDisablePauseOnFocusLoss.UseVisualStyleBackColor = true;
			// 
			// checkBoxControllerMapping
			// 
			this.checkBoxControllerMapping.AutoSize = true;
			this.checkBoxControllerMapping.Checked = true;
			this.checkBoxControllerMapping.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxControllerMapping.Location = new System.Drawing.Point(113, 323);
			this.checkBoxControllerMapping.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxControllerMapping.Name = "checkBoxControllerMapping";
			this.checkBoxControllerMapping.Size = new System.Drawing.Size(467, 19);
			this.checkBoxControllerMapping.TabIndex = 12;
			this.checkBoxControllerMapping.Text = "Fix inconsistent Controller Button Prompts and Mappings when remapping buttons";
			this.checkBoxControllerMapping.UseVisualStyleBackColor = true;
			// 
			// checkBoxArtsSupport
			// 
			this.checkBoxArtsSupport.AutoSize = true;
			this.checkBoxArtsSupport.Checked = true;
			this.checkBoxArtsSupport.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxArtsSupport.Location = new System.Drawing.Point(113, 350);
			this.checkBoxArtsSupport.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxArtsSupport.Name = "checkBoxArtsSupport";
			this.checkBoxArtsSupport.Size = new System.Drawing.Size(256, 19);
			this.checkBoxArtsSupport.TabIndex = 13;
			this.checkBoxArtsSupport.Text = "Fix invisible character in Arts Support cut-in";
			this.checkBoxArtsSupport.UseVisualStyleBackColor = true;
			// 
			// checkBoxForce0Kerning
			// 
			this.checkBoxForce0Kerning.AutoSize = true;
			this.checkBoxForce0Kerning.Location = new System.Drawing.Point(113, 376);
			this.checkBoxForce0Kerning.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxForce0Kerning.Name = "checkBoxForce0Kerning";
			this.checkBoxForce0Kerning.Size = new System.Drawing.Size(441, 19);
			this.checkBoxForce0Kerning.TabIndex = 14;
			this.checkBoxForce0Kerning.Text = "Adjust font spacing for use with HD Cuprum font from CS3 or HD Texture Pack";
			this.checkBoxForce0Kerning.UseVisualStyleBackColor = true;
			// 
			// checkBoxShowMouseCursor
			// 
			this.checkBoxShowMouseCursor.AutoSize = true;
			this.checkBoxShowMouseCursor.Location = new System.Drawing.Point(113, 428);
			this.checkBoxShowMouseCursor.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxShowMouseCursor.Name = "checkBoxShowMouseCursor";
			this.checkBoxShowMouseCursor.Size = new System.Drawing.Size(132, 19);
			this.checkBoxShowMouseCursor.TabIndex = 16;
			this.checkBoxShowMouseCursor.Text = "Show Mouse Cursor";
			this.checkBoxShowMouseCursor.UseVisualStyleBackColor = true;
			// 
			// checkBoxFixBattleScopeCrash
			// 
			this.checkBoxFixBattleScopeCrash.AutoSize = true;
			this.checkBoxFixBattleScopeCrash.Checked = true;
			this.checkBoxFixBattleScopeCrash.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxFixBattleScopeCrash.Location = new System.Drawing.Point(113, 503);
			this.checkBoxFixBattleScopeCrash.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxFixBattleScopeCrash.Name = "checkBoxFixBattleScopeCrash";
			this.checkBoxFixBattleScopeCrash.Size = new System.Drawing.Size(413, 19);
			this.checkBoxFixBattleScopeCrash.TabIndex = 19;
			this.checkBoxFixBattleScopeCrash.Text = "Try to fix Battle Scope crash (experimental; please report if it still happens)";
			this.checkBoxFixBattleScopeCrash.UseVisualStyleBackColor = true;
			// 
			// checkBoxForceXInput
			// 
			this.checkBoxForceXInput.AutoSize = true;
			this.checkBoxForceXInput.Location = new System.Drawing.Point(113, 478);
			this.checkBoxForceXInput.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.checkBoxForceXInput.Name = "checkBoxForceXInput";
			this.checkBoxForceXInput.Size = new System.Drawing.Size(415, 19);
			this.checkBoxForceXInput.TabIndex = 18;
			this.checkBoxForceXInput.Text = "Force all controllers as XInput (may fix issues with newer Xbox controllers)";
			this.checkBoxForceXInput.UseVisualStyleBackColor = true;
			// 
			// Sen2Form
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(623, 593);
			this.Controls.Add(this.checkBoxForceXInput);
			this.Controls.Add(this.checkBoxFixBattleScopeCrash);
			this.Controls.Add(this.checkBoxShowMouseCursor);
			this.Controls.Add(this.checkBoxForce0Kerning);
			this.Controls.Add(this.checkBoxArtsSupport);
			this.Controls.Add(this.checkBoxControllerMapping);
			this.Controls.Add(this.checkBoxDisablePauseOnFocusLoss);
			this.Controls.Add(this.checkBoxDisableMouseCam);
			this.Controls.Add(this.checkBoxFixVoiceFileLang);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxAssetPatches);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.label6);
			this.Controls.Add(this.checkBoxBgmEnqueueingLogic);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.checkBoxPatchAudioThread);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.checkBoxBattleAutoSkip);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			this.Name = "Sen2Form";
			this.Text = "Patch Cold Steel 2";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox checkBoxBattleAutoSkip;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.CheckBox checkBoxPatchAudioThread;
		private System.Windows.Forms.CheckBox checkBoxBgmEnqueueingLogic;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxAssetPatches;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.CheckBox checkBoxFixVoiceFileLang;
		private System.Windows.Forms.CheckBox checkBoxDisableMouseCam;
		private System.Windows.Forms.CheckBox checkBoxDisablePauseOnFocusLoss;
		private System.Windows.Forms.CheckBox checkBoxControllerMapping;
		private System.Windows.Forms.CheckBox checkBoxArtsSupport;
		private System.Windows.Forms.CheckBox checkBoxForce0Kerning;
		private System.Windows.Forms.CheckBox checkBoxShowMouseCursor;
		private System.Windows.Forms.CheckBox checkBoxFixBattleScopeCrash;
		private System.Windows.Forms.CheckBox checkBoxForceXInput;
	}
}