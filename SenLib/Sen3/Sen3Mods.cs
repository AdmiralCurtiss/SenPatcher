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
			bool swapBrokenMasterQuartzValuesForDisplay = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false,
			bool fixControllerMapping = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_3_exe(
					jp: i == 0,
					fixInGameButtonMappingValidity: fixInGameButtonMappingValidity,
					allowSwitchToNightmare: allowSwitchToNightmare,
					swapBrokenMasterQuartzValuesForDisplay: swapBrokenMasterQuartzValuesForDisplay,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixControllerMapping: fixControllerMapping
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods(bool allowSwitchToNightmare = false) {
			var f = new List<FileMod>();
			f.Add(new FileFixes.voice_opus_v00e0441());
			f.Add(new FileFixes.voice_opus_v00s2728());
			f.Add(new FileFixes.I_CVIS0061_pkg());
			f.Add(new FileFixes.I_CVIS1008_pkg());
			f.Add(new FileFixes.I_JMP009_pkg());
			f.Add(new FileFixes.t_mstqrt());
			f.Add(new FileFixes.t_item());
			f.Add(new FileFixes.t_magic());
			f.Add(new FileFixes.t_notecook());
			f.Add(new FileFixes.t_text(allowSwitchToNightmare));
			f.Add(new FileFixes.insa05());
			f.Add(new FileFixes.insa08());
			f.Add(new FileFixes.insa09());
			f.Add(new FileFixes.t0260_dat());
			f.Add(new FileFixes.f2000_dat());
			return f;
		}
	}
}
