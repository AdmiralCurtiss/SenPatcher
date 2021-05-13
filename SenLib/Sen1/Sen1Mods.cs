using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1 {
	public class Sen1Mods {
		public static List<FileMod> GetExecutableMods(
			bool removeTurboSkip = false,
			bool allowR2NotebookShortcut = false,
			int turboKey = 0x7,
			bool fixTextureIds = false,
			bool correctLanguageVoiceTables = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_exe(i == 0, removeTurboSkip, allowR2NotebookShortcut, turboKey, fixTextureIds, correctLanguageVoiceTables));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods() {
			var f = new List<FileMod>();
			f.Add(new FileFixes.scripts_scena_dat_us_t1000_dat());
			f.Add(new FileFixes.text_dat_us_t_magic_tbl());
			f.Add(new FileFixes.text_dat_t_voice_tbl());
			f.Add(new FileFixes.text_dat_us_t_voice_tbl());
			f.Add(new FileFixes.se_wav_ed8m2123_wav());
			f.Add(new FileFixes.se_wav_ed8m2150_wav());
			f.Add(new FileFixes.se_wav_ed8m4097_wav());
			f.Add(new FileFixes.se_wav_ed8m4217_wav());
			f.Add(new FileFixes.scripts_scena_dat_us_r0600_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0032_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t3500_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_heinrich_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_c0110_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0000_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1500_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_beryl_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_c0100_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_a0006_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_r0800_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0010_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0031_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0050_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0080_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1010_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_edel_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_laura_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_vandyck_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_m0040_dat());
			return f;
		}
	}
}
