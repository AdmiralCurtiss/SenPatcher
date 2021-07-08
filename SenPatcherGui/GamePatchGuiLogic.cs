using HyoutaUtils;
using SenLib;
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace SenPatcherGui {
	public class GamePatchClass {
		// input
		string Path;
		FileStorage Storage;
		List<FileMod> Mods;
		ProgressReporter Progress;

		// output
		public PatchResult Result = null;

		public GamePatchClass(string path, FileStorage storage, List<FileMod> mods) {
			Path = path;
			Storage = storage;
			Mods = mods;
		}

		public void SetReporter(ProgressReporter progress) {
			Progress = progress;
		}

		public void Run() {
			if (Progress == null) {
				Progress = new DummyProgressReporter();
			}

			try {
				Result = FileModExec.ExecuteMods(Path, Storage, Mods, Progress);
			} catch (Exception ex) {
				Progress.Error("Error while patching game files: " + ex.Message);
				Progress.Finish(false);
				return;
			}

			Progress.Message("Successfully patched game files.");
			Progress.Finish(true);
			SenLib.Logging.Flush();
		}

		public static void RunPatch(GamePatchClass patch) {
			var progressForm = new ProgressForm();
			patch.SetReporter(progressForm.GetProgressReporter());
			var thread = new System.Threading.Thread(patch.Run);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			var result = patch.Result;
			if (result.AllSuccessful) {
				MessageBox.Show(
					  "Successfully applied all selected patches.\n\n"
					+ "To remove the patches or a apply a different set of them, just run SenPatcher again.\n"
					+ "Note that 'senpatcher_rerun_revert_data.bin' in the game folder is required for this, so avoid deleting it.\n"
					+ "You can close SenPatcher now and just run the game as you normally would.",
					"Patching Success",
					MessageBoxButtons.OK
				);
				return;
			} else {
				MessageBox.Show(
					  "Failed applying " + result.FailedFiles + " of the selected " + result.TotalFiles + " patches.\n"
					+ "Please ensure SenPatcher is allowed to write to the game directory, then try again.\n"
					+ "It's also possible that the game data is corrupted or has been modified by other mods. "
					+ "If so, try verifying game files on Steam or GoG, or reinstall the game. "
					+ "Be sure to close and reopen SenPatcher before trying again in this case.",
					"Patching Error",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error
				);
			}
		}
	}

	public class GameUnpatchClass {
		// input
		string Path;
		FileStorage Storage;
		List<FileMod> Mods;
		ProgressReporter Progress;

		// output
		public PatchResult Result = null;

		public GameUnpatchClass(string path, FileStorage storage, List<FileMod> mods) {
			Path = path;
			Storage = storage;
			Mods = mods;
		}

		public void SetReporter(ProgressReporter progress) {
			Progress = progress;
		}

		public void Run() {
			if (Progress == null) {
				Progress = new DummyProgressReporter();
			}

			try {
				Result = FileModExec.RevertMods(Path, Storage, Mods, Progress);
			} catch (Exception ex) {
				Progress.Error("Error while removing patches from game files: " + ex.Message);
				Progress.Finish(false);
				return;
			}

			Progress.Message("Successfully restored game files.");
			Progress.Finish(true);
			SenLib.Logging.Flush();
		}

		public static void RunUnpatch(GameUnpatchClass patch) {
			var progressForm = new ProgressForm();
			patch.SetReporter(progressForm.GetProgressReporter());
			var thread = new System.Threading.Thread(patch.Run);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			var result = patch.Result;
			if (result.AllSuccessful) {
				MessageBox.Show(
					  "Successfully removed all patches.\n\n"
					+ "You can close SenPatcher now and just run the game as you normally would.",
					"Patching Success",
					MessageBoxButtons.OK
				);
				return;
			} else {
				MessageBox.Show(
					  "Failed removing " + result.FailedFiles + " of the selected " + result.TotalFiles + " patches.\n"
					+ "Please ensure SenPatcher is allowed to write to the game directory, then try again.\n"
					+ "It's also possible that the game data is corrupted or has been modified by other mods. "
					+ "If so, try verifying game files on Steam or GoG, or reinstall the game. "
					+ "Be sure to close and reopen SenPatcher before trying again in this case.",
					"Patching Error",
					MessageBoxButtons.OK,
					MessageBoxIcon.Error
				);
			}
		}
	}
}
