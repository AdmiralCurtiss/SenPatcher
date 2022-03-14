using HyoutaUtils;
using HyoutaUtils.Checksum;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
using SenLib.Sen3;
using SenLib.Sen4;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class MainForm : Form {
		public MainForm() {
			InitializeComponent();
			labelVersion.Text = "Version " + SenLib.Version.SenPatcherVersion;
		}

		private static string GetDefaultPathCS1() {
			if (Directory.Exists(Properties.Settings.Default.Sen1Path)) {
				return Properties.Settings.Default.Sen1Path;
			}
			if (Directory.Exists(SenCommonPaths.Sen1SteamDir)) {
				return SenCommonPaths.Sen1SteamDir;
			}
			if (Directory.Exists(SenCommonPaths.Sen1GalaxyDir)) {
				return SenCommonPaths.Sen1GalaxyDir;
			}
			return @"c:\";
		}

		private static string GetDefaultPathCS2() {
			if (Directory.Exists(Properties.Settings.Default.Sen2Path)) {
				return Properties.Settings.Default.Sen2Path;
			}
			if (Directory.Exists(SenCommonPaths.Sen2SteamDir)) {
				return SenCommonPaths.Sen2SteamDir;
			}
			if (Directory.Exists(SenCommonPaths.Sen2GalaxyDir)) {
				return SenCommonPaths.Sen2GalaxyDir;
			}
			return @"c:\";
		}

		private static string GetDefaultPathCS3() {
			if (Directory.Exists(Properties.Settings.Default.Sen3Path)) {
				return Properties.Settings.Default.Sen3Path;
			}
			if (Directory.Exists(SenCommonPaths.Sen3SteamDir)) {
				return SenCommonPaths.Sen3SteamDir;
			}
			return @"c:\";
		}

		private static string GetDefaultPathCS4() {
			if (Directory.Exists(Properties.Settings.Default.Sen4Path)) {
				return Properties.Settings.Default.Sen4Path;
			}
			if (Directory.Exists(SenCommonPaths.Sen4SteamDir)) {
				return SenCommonPaths.Sen4SteamDir;
			}
			return @"c:\";
		}

		private void buttonCS1Patch_Click(object sender, EventArgs e) {
			// this is not great UX wise, can we do better?
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GetDefaultPathCS1();
				d.FileName = "Sen1Launcher.exe";
				d.Filter = "CS1 root game directory (Sen1Launcher.exe)|Sen1Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs1GameDir(d.FileName);
				}
			}
		}

		private class Cs1GameInitClass {
			// input
			public string Sen1LauncherPath;
			public ProgressReporter Progress;

			// output
			public string Path;
			public FileStorage Storage;

			public bool ShouldProceedToPatchOptionWindow;

			public Cs1GameInitClass(string launcherPath, ProgressReporter progress) {
				Sen1LauncherPath = launcherPath;
				Progress = progress;
			}

			public void Cs1GameInit() {
				int CurrentProgress = 0;
				int TotalProgress = 4;
				bool shouldAutoCloseWindow = true;
				try {
					Progress.Message("Checking Sen1Launcher.exe...", CurrentProgress++, TotalProgress);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(Sen1LauncherPath)) {
						SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
						if (hash != new SHA1(0x8dde2b39f128179aul, 0x0beb3301cfd56a98ul, 0xc0f98a55u)) {
							Progress.Error("Selected file does not appear to be Sen1Launcher.exe of version 1.6.");
							Progress.Finish(false);
							return;
						}
					}
				} catch (Exception ex) {
					Progress.Error("Error while validating Sen1Launcher.exe: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				try {
					Path = System.IO.Path.GetDirectoryName(Sen1LauncherPath);
					Progress.Message("Checking if we have encoding errors in filenames...", CurrentProgress++, TotalProgress);
					if (FilenameFix.FixupIncorrectEncodingInFilenames(Path, 1, false, Progress)) {
						if (!FilenameFix.FixupIncorrectEncodingInFilenames(Path, 1, true, Progress)) {
							Progress.Error("Failed to fix encoding errors in filenames, attempting to proceed anyway...");
							shouldAutoCloseWindow = false;
						}
					}
					Progress.Message("Initializing patch data...", CurrentProgress++, TotalProgress);
					var files = Sen1KnownFiles.Files;
					Progress.Message("Initializing game data...", CurrentProgress++, TotalProgress);
					var storageInit = FileModExec.InitializeAndPersistFileStorage(Path, files, Progress);
					Storage = storageInit?.Storage;
					if (storageInit == null || storageInit.Errors.Count != 0) {
						shouldAutoCloseWindow = false;
					}
				} catch (Exception ex) {
					Progress.Error("Error while initializing CS1 patch/game data: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				ShouldProceedToPatchOptionWindow = Path != null && Storage != null;
				if (shouldAutoCloseWindow) {
					Progress.Message("Initialized CS1 data, proceeding to patch options...", CurrentProgress, TotalProgress);
				} else {
					Progress.Message("", CurrentProgress, TotalProgress);
					if (ShouldProceedToPatchOptionWindow) {
						Progress.Error(
							  "Encountered problems while initializing CS1 data. "
							+ "Closing this window will proceed to the patch options anyway, but be aware that some patches may not work correctly. "
							+ "It is recommended to verify the game files using Steam or GOG Galaxy's build-in feature to do so, or to reinstall the game. "
							+ "Please also ensure you're trying to patch a compatible version of the game. (XSEED release version 1.6; other game versions are not compatible)"
						);
					} else {
						Progress.Error(
							  "Unrecoverable issues while initializing CS1 data. "
							+ "Please ensure SenPatcher has read and write access to the selected game directory, then try again."
						);
					}
				}
				Progress.Finish(shouldAutoCloseWindow);
			}
		}

		private void OpenCs1GameDir(string launcherPath) {
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var init = new Cs1GameInitClass(launcherPath, progress);
			var thread = new System.Threading.Thread(init.Cs1GameInit);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			if (init.ShouldProceedToPatchOptionWindow) {
				Properties.Settings.Default.Sen1Path = init.Path;
				PropagateDefaultPathToUnset(init.Path);
				new Sen1Form(init.Path, init.Storage).ShowDialog();
			}
		}

		private void buttonCS2Patch_Click(object sender, EventArgs e) {
			// same UX problem as buttonCS1Patch_Click
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GetDefaultPathCS2();
				d.FileName = "Sen2Launcher.exe";
				d.Filter = "CS2 root game directory (Sen2Launcher.exe)|Sen2Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs2GameDir(d.FileName);
				}
			}
		}

		private class Cs2GameInitClass {
			// input
			public string Sen2LauncherPath;
			public ProgressReporter Progress;

			// output
			public string Path;
			public FileStorage Storage;

			public bool ShouldProceedToPatchOptionWindow;

			public Cs2GameInitClass(string launcherPath, ProgressReporter progress) {
				Sen2LauncherPath = launcherPath;
				Progress = progress;
			}

			public void Cs2GameInit() {
				int CurrentProgress = 0;
				int TotalProgress = 4;
				bool shouldAutoCloseWindow = true;
				try {
					Progress.Message("Checking Sen2Launcher.exe...", CurrentProgress++, TotalProgress);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(Sen2LauncherPath)) {
						SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
						if (hash != new SHA1(0x81024410cc1fd1b4ul, 0x62c600e0378714bdul, 0x7704b202u)) {
							Progress.Error("Selected file does not appear to be Sen2Launcher.exe of version 1.4, 1.4.1, or 1.4.2.");
							Progress.Finish(false);
							return;
						}
					}
				} catch (Exception ex) {
					Progress.Error("Error while validating Sen2Launcher.exe: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				try {
					Path = System.IO.Path.GetDirectoryName(Sen2LauncherPath);
					Progress.Message("Checking if we have encoding errors in filenames...", CurrentProgress++, TotalProgress);
					if (FilenameFix.FixupIncorrectEncodingInFilenames(Path, 2, false, Progress)) {
						if (!FilenameFix.FixupIncorrectEncodingInFilenames(Path, 2, true, Progress)) {
							Progress.Error("Failed to fix encoding errors in filenames, attempting to proceed anyway...");
							shouldAutoCloseWindow = false;
						}
					}
					Progress.Message("Initializing patch data...", CurrentProgress++, TotalProgress);
					var files = Sen2KnownFiles.Files;
					Progress.Message("Initializing game data...", CurrentProgress++, TotalProgress);
					var storageInit = FileModExec.InitializeAndPersistFileStorage(Path, files, Progress);
					Storage = storageInit?.Storage;
					if (storageInit == null || storageInit.Errors.Count != 0) {
						shouldAutoCloseWindow = false;
					}
				} catch (Exception ex) {
					Progress.Error("Error while initializing CS2 patch/game data: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				ShouldProceedToPatchOptionWindow = Path != null && Storage != null;
				if (shouldAutoCloseWindow) {
					Progress.Message("Initialized CS2 data, proceeding to patch options...", CurrentProgress, TotalProgress);
				} else {
					Progress.Message("", CurrentProgress, TotalProgress);
					if (ShouldProceedToPatchOptionWindow) {
						Progress.Error(
							  "Encountered problems while initializing CS2 data. "
							+ "Closing this window will proceed to the patch options anyway, but be aware that some patches may not work correctly. "
							+ "It is recommended to verify the game files using Steam or GOG Galaxy's build-in feature to do so, or to reinstall the game. "
							+ "Please also ensure you're trying to patch a compatible version of the game. (XSEED release version 1.4, 1.4.1, or 1.4.2; other game versions are not compatible)"
						);
					} else {
						Progress.Error(
							  "Unrecoverable issues while initializing CS2 data. "
							+ "Please ensure SenPatcher has read and write access to the selected game directory, then try again."
						);
					}
				}
				Progress.Finish(shouldAutoCloseWindow);
			}
		}

		private void OpenCs2GameDir(string launcherPath) {
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var init = new Cs2GameInitClass(launcherPath, progress);
			var thread = new System.Threading.Thread(init.Cs2GameInit);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			if (init.ShouldProceedToPatchOptionWindow) {
				Properties.Settings.Default.Sen2Path = init.Path;
				PropagateDefaultPathToUnset(init.Path);
				new Sen2Form(init.Path, init.Storage).ShowDialog();
			}
		}

		private void buttonCS1Sysdata_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Filter = "CS1 System Data File (" + SenCommonPaths.Sen1SystemDataFilename + ")|" + SenCommonPaths.Sen1SystemDataFilename + "|All files (*.*)|*.*";
				d.InitialDirectory = SenCommonPaths.Sen1SaveFolder;
				d.FileName = SenCommonPaths.Sen1SystemDataFilename;
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs1SystemData(d.FileName);
				}
			}
		}

		private void OpenCs1SystemData(string path) {
			if (!File.Exists(path)) {
				MessageBox.Show("No file found at " + path + ".");
				return;
			}

			Sen1SystemData data = null;
			using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
				if (fs.Length != Sen1SystemData.FileLength) {
					MessageBox.Show("Incorrect filesize for CS1 system data at " + path + ".");
					return;
				}
				data = new Sen1SystemData(fs.CopyToMemory(), HyoutaUtils.EndianUtils.Endianness.LittleEndian);
			}

			new Sen1SystemDataForm(data, path, HyoutaUtils.EndianUtils.Endianness.LittleEndian).ShowDialog();
		}

		private void buttonCS2Sysdata_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Filter = "CS2 System Data File (" + SenCommonPaths.Sen2SystemDataFilename + ")|" + SenCommonPaths.Sen2SystemDataFilename + "|All files (*.*)|*.*";
				d.InitialDirectory = SenCommonPaths.Sen2SaveFolder;
				d.FileName = SenCommonPaths.Sen2SystemDataFilename;
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs2SystemData(d.FileName);
				}
			}
		}

		private void OpenCs2SystemData(string path) {
			if (!File.Exists(path)) {
				MessageBox.Show("No file found at " + path + ".");
				return;
			}

			Sen2SystemData data = null;
			using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
				if (fs.Length != Sen2SystemData.FileLength) {
					MessageBox.Show("Incorrect filesize for CS2 system data at " + path + ".");
					return;
				}
				data = new Sen2SystemData(fs.CopyToMemory(), HyoutaUtils.EndianUtils.Endianness.LittleEndian);
			}

			new Sen2SystemDataForm(data, path, HyoutaUtils.EndianUtils.Endianness.LittleEndian).ShowDialog();
		}

		private void buttonCS3Patch_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GetDefaultPathCS3();
				d.FileName = "Sen3Launcher.exe";
				d.Filter = "CS3 root game directory (Sen3Launcher.exe)|Sen3Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs3GameDir(d.FileName);
				}
			}
		}

		private class Cs3GameInitClass {
			// input
			public string Sen3LauncherPath;
			public ProgressReporter Progress;

			// output
			public string Path;
			public FileStorage Storage;

			public bool ShouldProceedToPatchOptionWindow;

			public Cs3GameInitClass(string launcherPath, ProgressReporter progress) {
				Sen3LauncherPath = launcherPath;
				Progress = progress;
			}

			public void Cs3GameInit() {
				int CurrentProgress = 0;
				int TotalProgress = 3;
				bool shouldAutoCloseWindow = true;
				try {
					Progress.Message("Checking Sen3Launcher.exe...", CurrentProgress++, TotalProgress);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(Sen3LauncherPath)) {
						SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
						if (hash != new SHA1(0x21de3b088a5ddad7ul, 0xed1fdb8e40061497ul, 0xc248ca65u)) {
							Progress.Error("Selected file does not appear to be Sen3Launcher.exe of version 1.05.");
							Progress.Finish(false);
							return;
						}
					}
				} catch (Exception ex) {
					Progress.Error("Error while validating Sen3Launcher.exe: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				try {
					Path = System.IO.Path.GetDirectoryName(Sen3LauncherPath);
					Progress.Message("Initializing patch data...", CurrentProgress++, TotalProgress);
					var files = Sen3KnownFiles.Files;
					Progress.Message("Initializing game data...", CurrentProgress++, TotalProgress);
					var storageInit = FileModExec.InitializeAndPersistFileStorage(Path, files, Progress);
					Storage = storageInit?.Storage;
					if (storageInit == null || storageInit.Errors.Count != 0) {
						shouldAutoCloseWindow = false;
					}
				} catch (Exception ex) {
					Progress.Error("Error while initializing CS3 patch/game data: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				ShouldProceedToPatchOptionWindow = Path != null && Storage != null;
				if (shouldAutoCloseWindow) {
					Progress.Message("Initialized CS3 data, proceeding to patch options...", CurrentProgress, TotalProgress);
				} else {
					Progress.Message("", CurrentProgress, TotalProgress);
					if (ShouldProceedToPatchOptionWindow) {
						Progress.Error(
							  "Encountered problems while initializing CS3 data. "
							+ "Closing this window will proceed to the patch options anyway, but be aware that some patches may not work correctly. "
							+ "It is recommended to verify the game files using Steam or GOG Galaxy's build-in feature to do so, or to reinstall the game. "
							+ "Please also ensure you're trying to patch a compatible version of the game. (NISA release version 1.05; other game versions are not compatible, though if you own the EGS version I'd appreciate a report about file differences, if any!)"
						);
					} else {
						Progress.Error(
							  "Unrecoverable issues while initializing CS3 data. "
							+ "Please ensure SenPatcher has read and write access to the selected game directory, then try again."
						);
					}
				}
				Progress.Finish(shouldAutoCloseWindow);
			}
		}

		private void OpenCs3GameDir(string launcherPath) {
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var init = new Cs3GameInitClass(launcherPath, progress);
			var thread = new System.Threading.Thread(init.Cs3GameInit);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			if (init.ShouldProceedToPatchOptionWindow) {
				Properties.Settings.Default.Sen3Path = init.Path;
				PropagateDefaultPathToUnset(init.Path);
				new Sen3Form(init.Path, init.Storage).ShowDialog();
			}
		}

		private void buttonCS4Patch_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GetDefaultPathCS4();
				d.FileName = "Sen4Launcher.exe";
				d.Filter = "CS4 root game directory (Sen4Launcher.exe)|Sen4Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs4GameDir(d.FileName);
				}
			}
		}

		private class Cs4GameInitClass {
			// input
			public string Sen4LauncherPath;
			public ProgressReporter Progress;

			// output
			public string Path;
			public FileStorage Storage;

			public bool ShouldProceedToPatchOptionWindow;

			public Cs4GameInitClass(string launcherPath, ProgressReporter progress) {
				Sen4LauncherPath = launcherPath;
				Progress = progress;
			}

			public void Cs4GameInit() {
				int CurrentProgress = 0;
				int TotalProgress = 3;
				bool shouldAutoCloseWindow = true;
				try {
					Progress.Message("Checking Sen4Launcher.exe...", CurrentProgress++, TotalProgress);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(Sen4LauncherPath)) {
						SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
						if (hash != new SHA1(0x5f480136aa4c3b53ul, 0xadd422bf75b63350ul, 0xfa58d202u)) {
							Progress.Error("Selected file does not appear to be Sen4Launcher.exe of version 1.2.");
							Progress.Finish(false);
							return;
						}
					}
				} catch (Exception ex) {
					Progress.Error("Error while validating Sen4Launcher.exe: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				try {
					Path = System.IO.Path.GetDirectoryName(Sen4LauncherPath);
					Progress.Message("Initializing patch data...", CurrentProgress++, TotalProgress);
					var files = Sen4KnownFiles.Files;
					Progress.Message("Initializing game data...", CurrentProgress++, TotalProgress);
					var storageInit = FileModExec.InitializeAndPersistFileStorage(Path, files, Progress);
					Storage = storageInit?.Storage;
					if (storageInit == null || storageInit.Errors.Count != 0) {
						shouldAutoCloseWindow = false;
					}
				} catch (Exception ex) {
					Progress.Error("Error while initializing CS4 patch/game data: " + ex.Message);
					Progress.Finish(false);
					return;
				}

				ShouldProceedToPatchOptionWindow = Path != null && Storage != null;
				if (shouldAutoCloseWindow) {
					Progress.Message("Initialized CS4 data, proceeding to patch options...", CurrentProgress, TotalProgress);
				} else {
					Progress.Message("", CurrentProgress, TotalProgress);
					if (ShouldProceedToPatchOptionWindow) {
						Progress.Error(
							  "Encountered problems while initializing CS4 data. "
							+ "Closing this window will proceed to the patch options anyway, but be aware that some patches may not work correctly. "
							+ "It is recommended to verify the game files using Steam or GOG Galaxy's build-in feature to do so, or to reinstall the game. "
							+ "Please also ensure you're trying to patch a compatible version of the game. (NISA release version 1.2; other game versions are not compatible, though if you own the EGS version I'd appreciate a report about file differences, if any!)"
						);
					} else {
						Progress.Error(
							  "Unrecoverable issues while initializing CS4 data. "
							+ "Please ensure SenPatcher has read and write access to the selected game directory, then try again."
						);
					}
				}
				Progress.Finish(shouldAutoCloseWindow);
			}
		}

		private void OpenCs4GameDir(string launcherPath) {
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var init = new Cs4GameInitClass(launcherPath, progress);
			var thread = new System.Threading.Thread(init.Cs4GameInit);
			thread.Start();
			progressForm.ShowDialog();

			thread.Join();
			if (init.ShouldProceedToPatchOptionWindow) {
				Properties.Settings.Default.Sen4Path = init.Path;
				PropagateDefaultPathToUnset(init.Path);
				new Sen4Form(init.Path, init.Storage).ShowDialog();
			}
		}

		private void PropagateDefaultPathToUnset(string path) {
			try {
				string parent = Path.GetDirectoryName(path);
				if (Properties.Settings.Default.Sen1Path == "") {
					string p = Path.Combine(parent, "Trails of Cold Steel");
					if (Directory.Exists(p)) {
						Properties.Settings.Default.Sen1Path = p;
					}
				}
				if (Properties.Settings.Default.Sen2Path == "") {
					string p = Path.Combine(parent, "Trails of Cold Steel II");
					if (Directory.Exists(p)) {
						Properties.Settings.Default.Sen2Path = p;
					}
				}
				if (Properties.Settings.Default.Sen3Path == "") {
					string p = Path.Combine(parent, "The Legend of Heroes Trails of Cold Steel III");
					if (Directory.Exists(p)) {
						Properties.Settings.Default.Sen3Path = p;
					}
				}
				if (Properties.Settings.Default.Sen4Path == "") {
					string p = Path.Combine(parent, "The Legend of Heroes Trails of Cold Steel IV");
					if (Directory.Exists(p)) {
						Properties.Settings.Default.Sen4Path = p;
					}
				}
			} catch (Exception) { }
		}
	}
}
