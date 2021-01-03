using HyoutaUtils;
using SenLib;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen2 {
	public class Sen2PatchExec {
		public string BaseFolder { get; private set; }
		private FileStorage Storage;

		public Sen2PatchExec(string baseFolder, FileStorage storage) {
			BaseFolder = baseFolder;
			Storage = storage;
		}

		public static List<FileMod> GetMods(bool removeTurboSkip, bool patchAudioThread, int audioThreadDivisor, bool patchBgmQueueing, bool patchAssets) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_2_exe(i == 0, removeTurboSkip, patchAudioThread, audioThreadDivisor, patchBgmQueueing));
			}
			if (patchAssets) {
				f.Add(new FileFixes.text_dat_us_t_magic_tbl());
				f.Add(new FileFixes.scripts_book_dat_us_book04_dat());
			}
			return f;
		}

		public PatchResult ApplyPatches(List<FileMod> mods) {
			return FileModExec.ExecuteMods(BaseFolder, Storage, mods);
		}

		public PatchResult RevertPatches(List<FileMod> mods) {
			return FileModExec.RevertMods(BaseFolder, Storage, mods);
		}
	}
}
