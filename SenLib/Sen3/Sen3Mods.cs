using HyoutaUtils;
using SenLib;
using SenLib.Sen3;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen3 {
	public class Sen3Mods {
		public static List<FileMod> GetExecutableMods(
			bool fixInGameButtonMappingValidity = false,
			bool allowSwitchToNightmare = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_3_exe(
					jp: i == 0,
					fixInGameButtonMappingValidity: fixInGameButtonMappingValidity,
					allowSwitchToNightmare: allowSwitchToNightmare,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods() {
			var f = new List<FileMod>();
			f.Add(new FileFixes.voice_opus_v00e0441());
			f.Add(new FileFixes.voice_opus_v00s2728());
			f.Add(new FileFixes.I_CVIS0061_pkg());
			f.Add(new FileFixes.I_CVIS1008_pkg());
			f.Add(new FileFixes.I_JMP009_pkg());
			return f;
		}
	}
}
