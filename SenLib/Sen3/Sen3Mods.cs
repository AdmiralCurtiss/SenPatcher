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
			bool showMouseCursor = false,
			bool disablePauseOnFocusLoss = false,
			bool fixControllerMapping = false,
			bool forceXInput = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_3_exe(
					jp: i == 0,
					fixInGameButtonMappingValidity: fixInGameButtonMappingValidity,
					allowSwitchToNightmare: allowSwitchToNightmare,
					swapBrokenMasterQuartzValuesForDisplay: swapBrokenMasterQuartzValuesForDisplay,
					disableMouseCapture: disableMouseCapture,
					showMouseCursor: showMouseCursor,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixControllerMapping: fixControllerMapping,
					forceXInput: forceXInput
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
			f.Add(new FileFixes.t_jump());
			f.Add(new FileFixes.t_name());
			f.Add(new FileFixes.t_magic());
			f.Add(new FileFixes.t_notecook());
			f.Add(new FileFixes.t_place());
			f.Add(new FileFixes.t_text(allowSwitchToNightmare));
			f.Add(new FileFixes.voice_opus_ps4_103());
			f.Add(new FileFixes.t_vctiming_us());
			f.Add(new FileFixes.insa05());
			f.Add(new FileFixes.insa08());
			f.Add(new FileFixes.insa09());
			f.Add(new FileFixes.t0260_dat());
			f.Add(new FileFixes.f2000_dat());
			f.Add(new FileFixes.a0417_dat());
			f.Add(new FileFixes.c0200_dat());
			f.Add(new FileFixes.c0430_dat());
			f.Add(new FileFixes.c0830_dat());
			f.Add(new FileFixes.f0010_dat());
			f.Add(new FileFixes.m0600_dat());
			f.Add(new FileFixes.t3000_dat());
			f.Add(new FileFixes.t3200_dat());
			f.Add(new FileFixes.t3220_dat());
			f.Add(new FileFixes.m4004_dat());
			f.Add(new FileFixes.c3610_dat());
			f.Add(new FileFixes.r4290_dat());
			f.Add(new FileFixes.r0210_dat());
			f.Add(new FileFixes.m3000_dat());
			f.Add(new FileFixes.m0000_dat());
			f.Add(new FileFixes.m0100_dat());
			f.Add(new FileFixes.m0300_dat());
			f.Add(new FileFixes.t0410_dat());
			f.Add(new FileFixes.v0050_dat());
			f.Add(new FileFixes.v0010_dat());
			f.Add(new FileFixes.m3430_dat());
			f.Add(new FileFixes.m3420_dat());
			f.Add(new FileFixes.t0100_dat());
			f.Add(new FileFixes.t0200_dat());
			f.Add(new FileFixes.tk_patrick_dat());
			f.Add(new FileFixes.book07_dat());
			f.Add(new FileFixes.c0400_dat());
			f.Add(new FileFixes.c0250_dat());
			f.Add(new FileFixes.c0420_dat());
			f.Add(new FileFixes.c0820_dat());
			f.Add(new FileFixes.c3210_dat());
			f.Add(new FileFixes.c3010_dat());
			return f;
		}
	}
}
