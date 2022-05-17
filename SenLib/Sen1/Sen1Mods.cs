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
			bool correctLanguageVoiceTables = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false,
			bool fixArtsSupport = false,
			bool force0Kerning = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_exe(
					jp: i == 0,
					removeTurboSkip: removeTurboSkip,
					allowR2NotebookShortcut: allowR2NotebookShortcut,
					turboKey: turboKey,
					fixTextureIds: fixTextureIds,
					correctLanguageVoiceTables: correctLanguageVoiceTables,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixArtsSupport: fixArtsSupport,
					force0Kerning: force0Kerning
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods() {
			var f = new List<FileMod>();
			f.Add(new FileFixes.scripts_book_dat_us_book00_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book01_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book02_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book03_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_a0006_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_c0100_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_c0110_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_m0040_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_r0600_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_r0601_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_r0610_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_r0800_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0000_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0000c_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0010_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0020_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0031_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0032_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0050_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0060_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t0080_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1000_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1010_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1020_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1030_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1040_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1050_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1110_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t1500_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t3500_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_beryl_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_edel_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_heinrich_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_laura_dat());
			f.Add(new FileFixes.scripts_talk_dat_us_tk_vandyck_dat());
			f.Add(new FileFixes.text_dat_t_voice_tbl());
			f.Add(new FileFixes.text_dat_us_t_voice_tbl());
			f.Add(new FileFixes.text_dat_t_item_tbl());
			f.Add(new FileFixes.text_dat_us_t_item_tbl());
			f.Add(new FileFixes.text_dat_us_t_magic_tbl());
			f.Add(new FileFixes.text_dat_us_t_notecook_tbl());
			f.Add(new FileFixes.se_wav_ed8m2123_wav());
			f.Add(new FileFixes.se_wav_ed8m2150_wav());
			f.Add(new FileFixes.se_wav_ed8m4097_wav());
			f.Add(new FileFixes.se_wav_ed8m4217_wav());
			return f;
		}
	}
}
