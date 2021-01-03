namespace SenPatcherGui {
	partial class MainForm {
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
			this.buttonCS1Patch = new System.Windows.Forms.Button();
			this.labelVersion = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.buttonCS1Sysdata = new System.Windows.Forms.Button();
			this.buttonCS2Sysdata = new System.Windows.Forms.Button();
			this.buttonCS2Patch = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// buttonCS1Patch
			// 
			this.buttonCS1Patch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCS1Patch.Location = new System.Drawing.Point(12, 47);
			this.buttonCS1Patch.Name = "buttonCS1Patch";
			this.buttonCS1Patch.Size = new System.Drawing.Size(317, 21);
			this.buttonCS1Patch.TabIndex = 2;
			this.buttonCS1Patch.Text = "Patch game";
			this.buttonCS1Patch.UseVisualStyleBackColor = true;
			this.buttonCS1Patch.Click += new System.EventHandler(this.buttonCS1Patch_Click);
			// 
			// labelVersion
			// 
			this.labelVersion.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.labelVersion.AutoSize = true;
			this.labelVersion.Location = new System.Drawing.Point(242, 9);
			this.labelVersion.Name = "labelVersion";
			this.labelVersion.Size = new System.Drawing.Size(87, 13);
			this.labelVersion.TabIndex = 10;
			this.labelVersion.Text = "SenPatcher v0.0";
			this.labelVersion.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(12, 31);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(258, 13);
			this.label2.TabIndex = 15;
			this.label2.Text = "Trails of Cold Steel I: (XSEED PC release version 1.6)";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(9, 111);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(309, 13);
			this.label3.TabIndex = 16;
			this.label3.Text = "Trails of Cold Steel II: (XSEED PC release version 1.4.1 or 1.4.2)";
			// 
			// buttonCS1Sysdata
			// 
			this.buttonCS1Sysdata.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCS1Sysdata.Location = new System.Drawing.Point(12, 74);
			this.buttonCS1Sysdata.Name = "buttonCS1Sysdata";
			this.buttonCS1Sysdata.Size = new System.Drawing.Size(317, 21);
			this.buttonCS1Sysdata.TabIndex = 17;
			this.buttonCS1Sysdata.Text = "Edit System Save Data";
			this.buttonCS1Sysdata.UseVisualStyleBackColor = true;
			this.buttonCS1Sysdata.Click += new System.EventHandler(this.buttonCS1Sysdata_Click);
			// 
			// buttonCS2Sysdata
			// 
			this.buttonCS2Sysdata.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCS2Sysdata.Location = new System.Drawing.Point(12, 154);
			this.buttonCS2Sysdata.Name = "buttonCS2Sysdata";
			this.buttonCS2Sysdata.Size = new System.Drawing.Size(317, 21);
			this.buttonCS2Sysdata.TabIndex = 19;
			this.buttonCS2Sysdata.Text = "Edit System Save Data";
			this.buttonCS2Sysdata.UseVisualStyleBackColor = true;
			this.buttonCS2Sysdata.Click += new System.EventHandler(this.buttonCS2Sysdata_Click);
			// 
			// buttonCS2Patch
			// 
			this.buttonCS2Patch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCS2Patch.Location = new System.Drawing.Point(12, 127);
			this.buttonCS2Patch.Name = "buttonCS2Patch";
			this.buttonCS2Patch.Size = new System.Drawing.Size(317, 21);
			this.buttonCS2Patch.TabIndex = 18;
			this.buttonCS2Patch.Text = "Patch game";
			this.buttonCS2Patch.UseVisualStyleBackColor = true;
			this.buttonCS2Patch.Click += new System.EventHandler(this.buttonCS2Patch_Click);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(341, 184);
			this.Controls.Add(this.buttonCS2Sysdata);
			this.Controls.Add(this.buttonCS2Patch);
			this.Controls.Add(this.buttonCS1Sysdata);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.labelVersion);
			this.Controls.Add(this.buttonCS1Patch);
			this.Name = "MainForm";
			this.Text = "SenPatcher";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Button buttonCS1Patch;
		private System.Windows.Forms.Label labelVersion;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button buttonCS1Sysdata;
		private System.Windows.Forms.Button buttonCS2Sysdata;
		private System.Windows.Forms.Button buttonCS2Patch;
	}
}

