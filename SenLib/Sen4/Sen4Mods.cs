using HyoutaUtils;
using SenLib;
using SenLib.Sen4;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen4 {
	public class Sen4Mods {
		public static List<FileMod> GetExecutableMods(
			bool allowSwitchToNightmare = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false
		) {
			var f = new List<FileMod>();
			// TODO
			return f;
		}

		public static List<FileMod> GetAssetMods(bool allowSwitchToNightmare = false) {
			var f = new List<FileMod>();
			f.Add(new FileFixes.t_text(allowSwitchToNightmare));
			return f;
		}
	}
}
