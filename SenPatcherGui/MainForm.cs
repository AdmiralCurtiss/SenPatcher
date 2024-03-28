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
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SenPatcherGui {
	public partial class MainForm : Form {
		public MainForm() {
			InitializeComponent();
			labelVersion.Text = "Version " + SenLib.Version.SenPatcherVersion;
		}

		public static void ShowError(string msg, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			Logging.LogForwarded(string.Format("ERROR: {0}", msg), memberName, sourceFilePath, sourceLineNumber);
			MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
		}

		private void buttonCS1Patch_Click(object sender, EventArgs e) {
			// this is not great UX wise, can we do better?
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GamePaths.GetDefaultPathCS1();
				d.FileName = "Sen1Launcher.exe";
				d.Filter = "CS1 root game directory (Sen1Launcher.exe)|Sen1Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs1GameDir(d.FileName);
				}
			}
		}

		public string Cs1GameInit(string launcherPath) {
			try {
				Logging.Log("Checking Sen1Launcher.exe...");
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(launcherPath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x8dde2b39f128179aul, 0x0beb3301cfd56a98ul, 0xc0f98a55u)) {
						ShowError("Selected file does not appear to be Sen1Launcher.exe of version 1.6.");
						return null;
					}
				}
			} catch (Exception ex) {
				ShowError("Error while validating Sen1Launcher.exe: " + ex.Message);
				return null;
			}

			string path;
			try {
				path = System.IO.Path.GetDirectoryName(launcherPath);
				Logging.Log("Checking if we have encoding errors in filenames...");
				if (FilenameFix.FixupIncorrectEncodingInFilenames(path, 1, false, new DummyProgressReporter())) {
					if (!FilenameFix.FixupIncorrectEncodingInFilenames(path, 1, true, new DummyProgressReporter())) {
						Logging.Log("Failed to fix encoding errors in filenames, proceeding anyway...");
					}
				}
			} catch (Exception ex) {
				ShowError("Error while initializing CS1 patch/game data: " + ex.Message);
				return null;
			}

			return path;
		}

		private void OpenCs1GameDir(string launcherPath) {
			var path = Cs1GameInit(launcherPath);
			if (path != null) {
				Logging.Log("Launching CS1 patch window at " + path);
				Properties.Settings.Default.Sen1Path = path;
				new Sen1Form(path).ShowDialog();
			}
		}

		private void buttonCS2Patch_Click(object sender, EventArgs e) {
			// same UX problem as buttonCS1Patch_Click
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GamePaths.GetDefaultPathCS2();
				d.FileName = "Sen2Launcher.exe";
				d.Filter = "CS2 root game directory (Sen2Launcher.exe)|Sen2Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs2GameDir(d.FileName);
				}
			}
		}

		public string Cs2GameInit(string launcherPath) {
			try {
				Logging.Log("Checking Sen2Launcher.exe...");
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(launcherPath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x81024410cc1fd1b4ul, 0x62c600e0378714bdul, 0x7704b202u)) {
						ShowError("Selected file does not appear to be Sen2Launcher.exe of version 1.4, 1.4.1, or 1.4.2.");
						return null;
					}
				}
			} catch (Exception ex) {
				ShowError("Error while validating Sen2Launcher.exe: " + ex.Message);
				return null;
			}

			string path;
			try {
				path = System.IO.Path.GetDirectoryName(launcherPath);
				Logging.Log("Checking if we have encoding errors in filenames...");
				if (FilenameFix.FixupIncorrectEncodingInFilenames(path, 2, false, new DummyProgressReporter())) {
					if (!FilenameFix.FixupIncorrectEncodingInFilenames(path, 2, true, new DummyProgressReporter())) {
						Logging.Log("Failed to fix encoding errors in filenames, proceeding anyway...");
					}
				}
			} catch (Exception ex) {
				ShowError("Error while initializing CS2 patch/game data: " + ex.Message);
				return null;
			}

			return path;
		}

		private void OpenCs2GameDir(string launcherPath) {
			var path = Cs2GameInit(launcherPath);
			if (path != null) {
				Logging.Log("Launching CS2 patch window at " + path);
				Properties.Settings.Default.Sen2Path = path;
				new Sen2Form(path).ShowDialog();
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
				d.InitialDirectory = GamePaths.GetDefaultPathCS3();
				d.FileName = "Sen3Launcher.exe";
				d.Filter = "CS3 root game directory (Sen3Launcher.exe)|Sen3Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs3GameDir(d.FileName);
				}
			}
		}

		public string Cs3GameInit(string launcherPath) {
			try {
				Logging.Log("Checking Sen3Launcher.exe...");
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(launcherPath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x21de3b088a5ddad7ul, 0xed1fdb8e40061497ul, 0xc248ca65u)) {
						ShowError("Selected file does not appear to be Sen3Launcher.exe of version 1.06.");
						return null;
					}
				}
			} catch (Exception ex) {
				ShowError("Error while validating Sen3Launcher.exe: " + ex.Message);
				return null;
			}

			string path;
			try {
				path = System.IO.Path.GetDirectoryName(launcherPath);
			} catch (Exception ex) {
				ShowError("Error while initializing CS3 patch/game data: " + ex.Message);
				return null;
			}

			return path;
		}

		private void OpenCs3GameDir(string launcherPath) {
			var path = Cs3GameInit(launcherPath);
			if (path != null) {
				Logging.Log("Launching CS3 patch window at " + path);
				Properties.Settings.Default.Sen3Path = path;
				new Sen3Form(path).ShowDialog();
			}
		}

		private void buttonCS4Patch_Click(object sender, EventArgs e) {
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.CheckFileExists = false;
				d.ValidateNames = false;
				d.InitialDirectory = GamePaths.GetDefaultPathCS4();
				d.FileName = "Sen4Launcher.exe";
				d.Filter = "CS4 root game directory (Sen4Launcher.exe)|Sen4Launcher.exe|All files (*.*)|*.*";
				if (d.ShowDialog() == DialogResult.OK) {
					OpenCs4GameDir(d.FileName);
				}
			}
		}

		public string Cs4GameInit(string launcherPath) {
			try {
				Logging.Log("Checking Sen4Launcher.exe...");
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(launcherPath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x5f480136aa4c3b53ul, 0xadd422bf75b63350ul, 0xfa58d202u)) {
						ShowError("Selected file does not appear to be Sen4Launcher.exe of version 1.2.1.");
						return null;
					}
				}
			} catch (Exception ex) {
				ShowError("Error while validating Sen4Launcher.exe: " + ex.Message);
				return null;
			}

			string path;
			try {
				path = System.IO.Path.GetDirectoryName(launcherPath);
			} catch (Exception ex) {
				ShowError("Error while initializing CS4 patch/game data: " + ex.Message);
				return null;
			}

			return path;
		}

		private void OpenCs4GameDir(string launcherPath) {
			var path = Cs4GameInit(launcherPath);
			if (path != null) {
				Logging.Log("Launching CS4 patch window at " + path);
				Properties.Settings.Default.Sen4Path = path;
				new Sen4Form(path).ShowDialog();
			}
		}

		private static string LastToolboxDirectory = null;
		private static string InitialToolboxDirectory => LastToolboxDirectory ?? Directory.GetCurrentDirectory();

		private void menuItemExtractPkg_Click(object sender, EventArgs e) {
			string inpath;
			string outpath;
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Title = "Select PKG to unpack";
				d.InitialDirectory = InitialToolboxDirectory;
				d.Filter = "CS1/2/3/4 PKG file (*.pkg)|*.pkg|All files (*.*)|*.*";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				inpath = d.FileName;
			}
			using (SaveFileDialog d = new SaveFileDialog()) {
				d.Title = "Select target directory name (will be created)";
				d.InitialDirectory = Path.GetDirectoryName(inpath);
				d.FileName = Path.GetFileName(inpath + ".ex");
				d.Filter = "Target Directory|new directory name";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				outpath = d.FileName;
			}

			LastToolboxDirectory = Path.GetDirectoryName(inpath);
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var thread = new System.Threading.Thread(() => {
				try {
					Directory.CreateDirectory(outpath);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(inpath))
					using (var pkg = new Pkg(fs)) {
						for (int i = 0; i < pkg.Files.Count; ++i) {
							PkgFile file = pkg.Files[i];
							progress.Message("Extracting " + file.Filename + "...", i, pkg.Files.Count);
							using (var s = file.DataStream)
							using (var outfs = new FileStream(Path.Combine(outpath, file.Filename), FileMode.Create)) {
								StreamUtils.CopyStream(s, outfs);
							}
							if (progressForm.IsAlreadyClosed) {
								throw new Exception("Closed by user.");
							}
						}
					}
					progress.Finish(true);
				} catch (Exception ex) {
					progress.Error(ex.Message);
					progress.Finish(false);
				}
			});
			thread.Name = "ExtractPkg";
			thread.Start();
			progressForm.ShowDialog();
			thread.Join();
		}

		private void menuItemExtractPka_Click(object sender, EventArgs e) {
			string inpath;
			string outpath;
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Title = "Select PKA to unpack";
				d.InitialDirectory = InitialToolboxDirectory;
				d.Filter = "CS3/4 PKA file (*.pka)|*.pka|All files (*.*)|*.*";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				inpath = d.FileName;
			}
			using (SaveFileDialog d = new SaveFileDialog()) {
				d.Title = "Select target directory name (will be created)";
				d.InitialDirectory = Path.GetDirectoryName(inpath);
				d.FileName = Path.GetFileName(inpath + ".ex");
				d.Filter = "Target Directory|new directory name";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				outpath = d.FileName;
			}

			LastToolboxDirectory = Path.GetDirectoryName(inpath);
			var progressForm = new ProgressForm();
			var progress = progressForm.GetProgressReporter();
			var thread = new System.Threading.Thread(() => {
				try {
					Directory.CreateDirectory(outpath);
					using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(inpath))
					using (var pka = new Pka(fs)) {
						for (int i = 0; i < pka.PkgCount; ++i) {
							string pkgName = pka.GetPkgName(i);
							progress.Message("Extracting " + pkgName + "...", i, pka.PkgCount);
							using (var pkgStream = pka.BuildPkgToMemory(i))
							using (var outfs = new FileStream(Path.Combine(outpath, pkgName), FileMode.Create)) {
								StreamUtils.CopyStream(pkgStream, outfs);
							}
							if (progressForm.IsAlreadyClosed) {
								throw new Exception("Closed by user.");
							}
						}
					}
					progress.Finish(true);
				} catch (Exception ex) {
					progress.Error(ex.Message);
					progress.Finish(false);
				}
			});
			thread.Name = "ExtractPka";
			thread.Start();
			progressForm.ShowDialog();
			thread.Join();
		}

		private void menuItemFixSaveChecksumCS4_Click(object sender, EventArgs e) {
			string inpath;
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Title = "Select CS4 save file to recalculate checksum for";
				d.InitialDirectory = Path.Combine(SenCommonPaths.SavedGamesFolder, @"FALCOM\ed8_psv4");
				d.Filter = "CS4 save file (*.dat)|*.dat|All files (*.*)|*.*";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				inpath = d.FileName;
			}

			using (var fs = new FileStream(inpath, FileMode.Open, FileAccess.ReadWrite)) {
				var type = SenLib.Sen4.Save.IdentifySaveFile(fs);
				if (type == null) {
					MessageBox.Show("This does not appear to be a CS4 save file.");
					return;
				}
				try {
					if (SenLib.Sen4.Save.FixSaveChecksum(fs)) {
						MessageBox.Show("Checksum updated.");
					} else {
						MessageBox.Show("Failed updating checksum.");
					}
				} catch (Exception ex) {
					MessageBox.Show("Failed updating checksum: " + ex.Message);
				}
			}
		}

		private void menuItemDecompressType1LE(object sender, EventArgs e) {
			string inpath;
			string outpath;
			using (OpenFileDialog d = new OpenFileDialog()) {
				d.Title = "Select Type 1 compressed file";
				d.InitialDirectory = InitialToolboxDirectory;
				d.Filter = "All files (*.*)|*.*";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				inpath = d.FileName;
			}
			using (SaveFileDialog d = new SaveFileDialog()) {
				d.Title = "Select target file";
				d.InitialDirectory = Path.GetDirectoryName(inpath);
				d.FileName = Path.GetFileName(inpath + ".dec");
				d.Filter = "All files (*.*)|*.*";
				if (d.ShowDialog() != DialogResult.OK) {
					return;
				}
				outpath = d.FileName;
			}

			LastToolboxDirectory = Path.GetDirectoryName(inpath);
			using (var fs = new FileStream(inpath, FileMode.Open, FileAccess.ReadWrite)) {
				try {
					var output = SenLib.PkgFile.DecompressType1(fs, EndianUtils.Endianness.LittleEndian);
					System.IO.File.WriteAllBytes(outpath, output);
					MessageBox.Show("Wrote " + output.Length + " bytes.");
				} catch (Exception ex) {
					MessageBox.Show("Decompression failed: " + ex.Message);
				}
			}
		}
	}
}
