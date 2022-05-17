using HyoutaUtils;
using SenLib;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2 {
	public enum Sen2Version {
		v14, v141, v142
	}

	public class Sen2Mods {
		public static List<FileMod> GetExecutableMods(
			Sen2Version version,
			bool removeTurboSkip = false,
			bool patchAudioThread = false,
			int audioThreadDivisor = 1000,
			bool patchBgmQueueing = false,
			bool correctLanguageVoiceTables = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false,
			bool fixControllerMapping = false,
			bool fixArtsSupport = false,
			bool force0Kerning = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_2_exe(
					version,
					jp: i == 0,
					removeTurboSkip: removeTurboSkip,
					patchAudioThread: patchAudioThread,
					audioThreadDivisor: audioThreadDivisor,
					patchBgmQueueing: patchBgmQueueing,
					correctLanguageVoiceTables: correctLanguageVoiceTables,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss,
					fixControllerMapping: fixControllerMapping,
					fixArtsSupport: fixArtsSupport,
					force0Kerning: force0Kerning
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods(Sen2Version version) {
			var f = new List<FileMod>();
			f.Add(new FileFixes.text_dat_us_t_magic_tbl());
			f.Add(new FileFixes.text_dat_us_t_item_tbl());
			f.Add(new FileFixes.text_dat_us_t_notecook_tbl());
			f.Add(new FileFixes.text_dat_us_t_voice_tbl());
			f.Add(new FileFixes.scripts_book_dat_us_book00_dat());
			//f.Add(new FileFixes.scripts_book_dat_us_book01_dat());
			//f.Add(new FileFixes.scripts_book_dat_us_book02_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book03_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book04_dat());
			f.Add(new FileFixes.scripts_book_dat_us_book05_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_e7101_dat());
			f.Add(new FileFixes.scripts_scena_dat_us_t4080_dat());
			f.Add(new FileFixes.scripts_scena_asm_t1001_tbl(version));
			f.Add(new FileFixes.scripts_scena_dat_t1001_dat(version));
			f.Add(new FileFixes.scripts_scena_dat_us_t1001_dat(version));
			return f;
		}

		public static bool CanRevertTo(Sen2Version version, FileStorage storage) {
			var exemods = GetExecutableMods(version);
			foreach (var exemod in exemods) {
				if (exemod.TryRevert(storage) == null) {
					return false;
				}
			}
			return (new FileFixes.scripts_scena_asm_t1001_tbl(version).TryRevert(storage)) != null
				&& (new FileFixes.scripts_scena_dat_t1001_dat(version).TryRevert(storage)) != null
				&& (new FileFixes.scripts_scena_dat_us_t1001_dat(version).TryRevert(storage)) != null;
		}
	}
}
