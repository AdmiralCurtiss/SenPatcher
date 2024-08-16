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
			this.label2 = new System.Windows.Forms.Label();
			this.checkBoxSkipMovies = new System.Windows.Forms.CheckBox();
			this.labelFile = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonPatch = new System.Windows.Forms.Button();
			this.buttonUnpatch = new System.Windows.Forms.Button();
			this.checkBoxSkipLogos = new System.Windows.Forms.CheckBox();
			this.buttonAssetFixDetails = new System.Windows.Forms.Button();
			this.comboBoxGameLanguage = new System.Windows.Forms.ComboBox();
			this.label4 = new System.Windows.Forms.Label();
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
			// checkBoxSkipMovies
			// 
			this.checkBoxSkipMovies.AutoSize = true;
			this.checkBoxSkipMovies.Checked = true;
			this.checkBoxSkipMovies.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBoxSkipMovies.Location = new System.Drawing.Point(97, 122);
			this.checkBoxSkipMovies.Name = "checkBoxSkipMovies";
			this.checkBoxSkipMovies.Size = new System.Drawing.Size(96, 17);
			this.checkBoxSkipMovies.TabIndex = 6;
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
			this.buttonPatch.Location = new System.Drawing.Point(12, 178);
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
			this.checkBoxSkipLogos.Location = new System.Drawing.Point(97, 99);
			this.checkBoxSkipLogos.Name = "checkBoxSkipLogos";
			this.checkBoxSkipLogos.Size = new System.Drawing.Size(110, 17);
			this.checkBoxSkipLogos.TabIndex = 4;
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
			this.comboBoxGameLanguage.Location = new System.Drawing.Point(187, 144);
			this.comboBoxGameLanguage.Name = "comboBoxGameLanguage";
			this.comboBoxGameLanguage.Size = new System.Drawing.Size(126, 21);
			this.comboBoxGameLanguage.TabIndex = 22;
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(94, 147);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(85, 13);
			this.label4.TabIndex = 21;
			this.label4.Text = "Game language:";
			// 
			// TXForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(534, 231);
			this.Controls.Add(this.comboBoxGameLanguage);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.buttonAssetFixDetails);
			this.Controls.Add(this.checkBoxSkipLogos);
			this.Controls.Add(this.buttonUnpatch);
			this.Controls.Add(this.buttonPatch);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.checkBoxSkipMovies);
			this.Controls.Add(this.labelFile);
			this.Controls.Add(this.label1);
			this.Name = "TXForm";
			this.Text = "Patch Tokyo Xanadu eX+";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox checkBoxSkipMovies;
		private System.Windows.Forms.Label labelFile;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonPatch;
		private System.Windows.Forms.Button buttonUnpatch;
		private System.Windows.Forms.CheckBox checkBoxSkipLogos;
		private System.Windows.Forms.Button buttonAssetFixDetails;
		private System.Windows.Forms.ComboBox comboBoxGameLanguage;
		private System.Windows.Forms.Label label4;
	}
}