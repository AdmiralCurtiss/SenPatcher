using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using System;
using System.Collections.Generic;
using System.IO;

namespace SenLib.Sen1 {
	public class Sen1PatchExec {
		public string BaseFolder { get; private set; }
		private FileStorage Storage;

		public Sen1PatchExec(string baseFolder, FileStorage storage) {
			BaseFolder = baseFolder;
			Storage = storage;
		}

		public static List<FileMod> GetMods(bool removeTurboSkip, bool allowR2NotebookShortcut, int turboKey, bool fixTextureIds, bool patchAssets) {
			var f = new List<FileMod>();
			for (int i = 0; i < 2; ++i) {
				f.Add(new FileFixes.ed8_exe(i == 0, removeTurboSkip, allowR2NotebookShortcut, turboKey, fixTextureIds));
			}
			if (patchAssets) {
				f.Add(new FileFixes.scripts_scena_dat_us_t1000_dat());
				f.Add(new FileFixes.text_dat_us_t_magic_tbl());
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
