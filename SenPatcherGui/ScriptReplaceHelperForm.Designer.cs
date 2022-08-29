namespace SenPatcherGui {
	partial class ScriptReplaceHelperForm {
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
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.textBoxFile = new System.Windows.Forms.TextBox();
			this.textBoxSearch = new System.Windows.Forms.TextBox();
			this.textBoxReplace = new System.Windows.Forms.TextBox();
			this.textBoxOutput = new System.Windows.Forms.TextBox();
			this.buttonGo = new System.Windows.Forms.Button();
			this.comboBoxGame = new System.Windows.Forms.ComboBox();
			this.comboBoxEncoding = new System.Windows.Forms.ComboBox();
			this.comboBoxEndian = new System.Windows.Forms.ComboBox();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(12, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(23, 13);
			this.label1.TabIndex = 0;
			this.label1.Text = "File";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(12, 41);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(41, 13);
			this.label2.TabIndex = 1;
			this.label2.Text = "Search";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(12, 67);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(47, 13);
			this.label3.TabIndex = 2;
			this.label3.Text = "Replace";
			// 
			// textBoxFile
			// 
			this.textBoxFile.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.textBoxFile.Location = new System.Drawing.Point(65, 12);
			this.textBoxFile.Name = "textBoxFile";
			this.textBoxFile.Size = new System.Drawing.Size(646, 20);
			this.textBoxFile.TabIndex = 3;
			// 
			// textBoxSearch
			// 
			this.textBoxSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.textBoxSearch.Location = new System.Drawing.Point(65, 38);
			this.textBoxSearch.Name = "textBoxSearch";
			this.textBoxSearch.Size = new System.Drawing.Size(646, 20);
			this.textBoxSearch.TabIndex = 4;
			// 
			// textBoxReplace
			// 
			this.textBoxReplace.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.textBoxReplace.Location = new System.Drawing.Point(65, 64);
			this.textBoxReplace.Name = "textBoxReplace";
			this.textBoxReplace.Size = new System.Drawing.Size(646, 20);
			this.textBoxReplace.TabIndex = 5;
			// 
			// textBoxOutput
			// 
			this.textBoxOutput.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.textBoxOutput.Location = new System.Drawing.Point(12, 119);
			this.textBoxOutput.Multiline = true;
			this.textBoxOutput.Name = "textBoxOutput";
			this.textBoxOutput.Size = new System.Drawing.Size(699, 387);
			this.textBoxOutput.TabIndex = 6;
			// 
			// buttonGo
			// 
			this.buttonGo.Location = new System.Drawing.Point(12, 90);
			this.buttonGo.Name = "buttonGo";
			this.buttonGo.Size = new System.Drawing.Size(177, 23);
			this.buttonGo.TabIndex = 7;
			this.buttonGo.Text = "Go";
			this.buttonGo.UseVisualStyleBackColor = true;
			this.buttonGo.Click += new System.EventHandler(this.buttonGo_Click);
			// 
			// comboBoxGame
			// 
			this.comboBoxGame.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxGame.FormattingEnabled = true;
			this.comboBoxGame.Location = new System.Drawing.Point(211, 92);
			this.comboBoxGame.Name = "comboBoxGame";
			this.comboBoxGame.Size = new System.Drawing.Size(65, 21);
			this.comboBoxGame.TabIndex = 8;
			// 
			// comboBoxEncoding
			// 
			this.comboBoxEncoding.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxEncoding.FormattingEnabled = true;
			this.comboBoxEncoding.Location = new System.Drawing.Point(282, 92);
			this.comboBoxEncoding.Name = "comboBoxEncoding";
			this.comboBoxEncoding.Size = new System.Drawing.Size(61, 21);
			this.comboBoxEncoding.TabIndex = 9;
			// 
			// comboBoxEndian
			// 
			this.comboBoxEndian.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxEndian.FormattingEnabled = true;
			this.comboBoxEndian.Location = new System.Drawing.Point(349, 92);
			this.comboBoxEndian.Name = "comboBoxEndian";
			this.comboBoxEndian.Size = new System.Drawing.Size(92, 21);
			this.comboBoxEndian.TabIndex = 10;
			// 
			// ScriptReplaceHelperForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(723, 518);
			this.Controls.Add(this.comboBoxEndian);
			this.Controls.Add(this.comboBoxEncoding);
			this.Controls.Add(this.comboBoxGame);
			this.Controls.Add(this.buttonGo);
			this.Controls.Add(this.textBoxOutput);
			this.Controls.Add(this.textBoxReplace);
			this.Controls.Add(this.textBoxSearch);
			this.Controls.Add(this.textBoxFile);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Name = "ScriptReplaceHelperForm";
			this.Text = "ScriptReplaceHelperForm";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox textBoxFile;
		private System.Windows.Forms.TextBox textBoxSearch;
		private System.Windows.Forms.TextBox textBoxReplace;
		private System.Windows.Forms.TextBox textBoxOutput;
		private System.Windows.Forms.Button buttonGo;
		private System.Windows.Forms.ComboBox comboBoxGame;
		private System.Windows.Forms.ComboBox comboBoxEncoding;
		private System.Windows.Forms.ComboBox comboBoxEndian;
	}
}