﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	static class Program {
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() {
			SenLib.Logging.LogToFile = true;
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new ScriptReplaceHelperForm());
			SenLib.Logging.Flush();
			try {
				Properties.Settings.Default.Save();
			} catch (Exception) { }
		}
	}
}
