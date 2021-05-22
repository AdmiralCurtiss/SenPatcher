using HyoutaUtils;
using SenLib;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2 {
	public class Sen2Mods {
		public static List<FileMod> GetExecutableMods(
			bool removeTurboSkip = false,
			bool patchAudioThread = false,
			int audioThreadDivisor = 1000,
			bool patchBgmQueueing = false,
			bool correctLanguageVoiceTables = false,
			bool disableMouseCapture = false,
			bool disablePauseOnFocusLoss = false
		) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_2_exe(
					jp: i == 0,
					removeTurboSkip: removeTurboSkip,
					patchAudioThread: patchAudioThread,
					audioThreadDivisor: audioThreadDivisor,
					patchBgmQueueing: patchBgmQueueing,
					correctLanguageVoiceTables: correctLanguageVoiceTables,
					disableMouseCapture: disableMouseCapture,
					disablePauseOnFocusLoss: disablePauseOnFocusLoss
				));
			}
			return f;
		}

		public static List<FileMod> GetAssetMods() {
			var f = new List<FileMod>();
			f.Add(new FileFixes.text_dat_us_t_magic_tbl());
			f.Add(new FileFixes.scripts_book_dat_us_book04_dat());
			return f;
		}
	}
}
