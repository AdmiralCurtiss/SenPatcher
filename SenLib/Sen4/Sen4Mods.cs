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
			bool showMouseCursor = false,
			bool disablePauseOnFocusLoss = false,
			bool separateSwapConfirmCancelOption = false,
			bool defaultSwapConfirmCancelOptionOn = false,
			bool fixSwappedButtonsWhenDynamicPromptsOff = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_4_exe(
					jp: i == 0,
					allowSwitchToNightmare: allowSwitchToNightmare,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					separateSwapConfirmCancelOption: separateSwapConfirmCancelOption,
					defaultSwapConfirmCancelOptionOn: defaultSwapConfirmCancelOptionOn,
					fixSwappedButtonsWhenDynamicPromptsOff: fixSwappedButtonsWhenDynamicPromptsOff
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods(bool allowSwitchToNightmare = false) {
			var f = new List<FileMod>();
			f.Add(new FileFixes.t_item());
			f.Add(new FileFixes.t_mstqrt());
			f.Add(new FileFixes.t_text(allowSwitchToNightmare));
			f.Add(new FileFixes.t3600_dat());
			f.Add(new FileFixes.m9031_dat());
			f.Add(new FileFixes.f4200_dat());
			return f;
		}
	}
}
