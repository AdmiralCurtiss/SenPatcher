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
			this.buttonManuallySelect = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.buttonCs1SteamEn = new System.Windows.Forms.Button();
			this.buttonCs1SteamJp = new System.Windows.Forms.Button();
			this.buttonCs1GalaxyJp = new System.Windows.Forms.Button();
			this.buttonCs1GalaxyEn = new System.Windows.Forms.Button();
			this.buttonCs2SteamJp = new System.Windows.Forms.Button();
			this.buttonCs2SteamEn = new System.Windows.Forms.Button();
			this.buttonCs2GalaxyJp = new System.Windows.Forms.Button();
			this.buttonCs2GalaxyEn = new System.Windows.Forms.Button();
			this.labelVersion = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// buttonManuallySelect
			// 
			this.buttonManuallySelect.Location = new System.Drawing.Point(16, 266);
			this.buttonManuallySelect.Name = "buttonManuallySelect";
			this.buttonManuallySelect.Size = new System.Drawing.Size(449, 50);
			this.buttonManuallySelect.TabIndex = 0;
			this.buttonManuallySelect.Text = "Manually select executable instead... (CS1 or CS2, any language)";
			this.buttonManuallySelect.UseVisualStyleBackColor = true;
			this.buttonManuallySelect.Click += new System.EventHandler(this.buttonManuallySelect_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(314, 65);
			this.label1.TabIndex = 1;
			this.label1.Text = "Patcher for the XSEED PC releases of Trails of Cold Steel I and II\r\n\r\nSupports:\r\n" +
    "- CS1 version 1.6\r\n- CS2 versions 1.4.1 and 1.4.2";
			// 
			// buttonCs1SteamEn
			// 
			this.buttonCs1SteamEn.Location = new System.Drawing.Point(16, 106);
			this.buttonCs1SteamEn.Name = "buttonCs1SteamEn";
			this.buttonCs1SteamEn.Size = new System.Drawing.Size(213, 34);
			this.buttonCs1SteamEn.TabIndex = 2;
			this.buttonCs1SteamEn.Text = "Find Steam version of CS1 (English)";
			this.buttonCs1SteamEn.UseVisualStyleBackColor = true;
			this.buttonCs1SteamEn.Click += new System.EventHandler(this.buttonCs1SteamEn_Click);
			// 
			// buttonCs1SteamJp
			// 
			this.buttonCs1SteamJp.Location = new System.Drawing.Point(235, 106);
			this.buttonCs1SteamJp.Name = "buttonCs1SteamJp";
			this.buttonCs1SteamJp.Size = new System.Drawing.Size(230, 34);
			this.buttonCs1SteamJp.TabIndex = 3;
			this.buttonCs1SteamJp.Text = "Find Steam version of CS1 (Japanese)";
			this.buttonCs1SteamJp.UseVisualStyleBackColor = true;
			this.buttonCs1SteamJp.Click += new System.EventHandler(this.buttonCs1SteamJp_Click);
			// 
			// buttonCs1GalaxyJp
			// 
			this.buttonCs1GalaxyJp.Location = new System.Drawing.Point(235, 146);
			this.buttonCs1GalaxyJp.Name = "buttonCs1GalaxyJp";
			this.buttonCs1GalaxyJp.Size = new System.Drawing.Size(230, 34);
			this.buttonCs1GalaxyJp.TabIndex = 5;
			this.buttonCs1GalaxyJp.Text = "Find GOG Galaxy version of CS1 (Japanese)";
			this.buttonCs1GalaxyJp.UseVisualStyleBackColor = true;
			this.buttonCs1GalaxyJp.Click += new System.EventHandler(this.buttonCs1GalaxyJp_Click);
			// 
			// buttonCs1GalaxyEn
			// 
			this.buttonCs1GalaxyEn.Location = new System.Drawing.Point(16, 146);
			this.buttonCs1GalaxyEn.Name = "buttonCs1GalaxyEn";
			this.buttonCs1GalaxyEn.Size = new System.Drawing.Size(213, 34);
			this.buttonCs1GalaxyEn.TabIndex = 4;
			this.buttonCs1GalaxyEn.Text = "Find GOG Galaxy version of CS1 (English)";
			this.buttonCs1GalaxyEn.UseVisualStyleBackColor = true;
			this.buttonCs1GalaxyEn.Click += new System.EventHandler(this.buttonCs1GalaxyEn_Click);
			// 
			// buttonCs2SteamJp
			// 
			this.buttonCs2SteamJp.Location = new System.Drawing.Point(235, 186);
			this.buttonCs2SteamJp.Name = "buttonCs2SteamJp";
			this.buttonCs2SteamJp.Size = new System.Drawing.Size(230, 34);
			this.buttonCs2SteamJp.TabIndex = 7;
			this.buttonCs2SteamJp.Text = "Find Steam version of CS2 (Japanese)";
			this.buttonCs2SteamJp.UseVisualStyleBackColor = true;
			this.buttonCs2SteamJp.Click += new System.EventHandler(this.buttonCs2SteamJp_Click);
			// 
			// buttonCs2SteamEn
			// 
			this.buttonCs2SteamEn.Location = new System.Drawing.Point(16, 186);
			this.buttonCs2SteamEn.Name = "buttonCs2SteamEn";
			this.buttonCs2SteamEn.Size = new System.Drawing.Size(213, 34);
			this.buttonCs2SteamEn.TabIndex = 6;
			this.buttonCs2SteamEn.Text = "Find Steam version of CS2 (English)";
			this.buttonCs2SteamEn.UseVisualStyleBackColor = true;
			this.buttonCs2SteamEn.Click += new System.EventHandler(this.buttonCs2SteamEn_Click);
			// 
			// buttonCs2GalaxyJp
			// 
			this.buttonCs2GalaxyJp.Location = new System.Drawing.Point(235, 226);
			this.buttonCs2GalaxyJp.Name = "buttonCs2GalaxyJp";
			this.buttonCs2GalaxyJp.Size = new System.Drawing.Size(230, 34);
			this.buttonCs2GalaxyJp.TabIndex = 9;
			this.buttonCs2GalaxyJp.Text = "Find GOG Galaxy version of CS2 (Japanese)";
			this.buttonCs2GalaxyJp.UseVisualStyleBackColor = true;
			this.buttonCs2GalaxyJp.Click += new System.EventHandler(this.buttonCs2GalaxyJp_Click);
			// 
			// buttonCs2GalaxyEn
			// 
			this.buttonCs2GalaxyEn.Location = new System.Drawing.Point(16, 226);
			this.buttonCs2GalaxyEn.Name = "buttonCs2GalaxyEn";
			this.buttonCs2GalaxyEn.Size = new System.Drawing.Size(213, 34);
			this.buttonCs2GalaxyEn.TabIndex = 8;
			this.buttonCs2GalaxyEn.Text = "Find GOG Galaxy version of CS2 (English)";
			this.buttonCs2GalaxyEn.UseVisualStyleBackColor = true;
			this.buttonCs2GalaxyEn.Click += new System.EventHandler(this.buttonCs2GalaxyEn_Click);
			// 
			// labelVersion
			// 
			this.labelVersion.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.labelVersion.AutoSize = true;
			this.labelVersion.Location = new System.Drawing.Point(437, 9);
			this.labelVersion.Name = "labelVersion";
			this.labelVersion.Size = new System.Drawing.Size(28, 13);
			this.labelVersion.TabIndex = 10;
			this.labelVersion.Text = "v0.0";
			this.labelVersion.TextAlign = System.Drawing.ContentAlignment.TopRight;
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(477, 325);
			this.Controls.Add(this.labelVersion);
			this.Controls.Add(this.buttonCs2GalaxyJp);
			this.Controls.Add(this.buttonCs2GalaxyEn);
			this.Controls.Add(this.buttonCs2SteamJp);
			this.Controls.Add(this.buttonCs2SteamEn);
			this.Controls.Add(this.buttonCs1GalaxyJp);
			this.Controls.Add(this.buttonCs1GalaxyEn);
			this.Controls.Add(this.buttonCs1SteamJp);
			this.Controls.Add(this.buttonCs1SteamEn);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.buttonManuallySelect);
			this.Name = "MainForm";
			this.Text = "SenPatcher";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button buttonManuallySelect;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button buttonCs1SteamEn;
		private System.Windows.Forms.Button buttonCs1SteamJp;
		private System.Windows.Forms.Button buttonCs1GalaxyJp;
		private System.Windows.Forms.Button buttonCs1GalaxyEn;
		private System.Windows.Forms.Button buttonCs2SteamJp;
		private System.Windows.Forms.Button buttonCs2SteamEn;
		private System.Windows.Forms.Button buttonCs2GalaxyJp;
		private System.Windows.Forms.Button buttonCs2GalaxyEn;
		private System.Windows.Forms.Label labelVersion;
	}
}

