using HyoutaUtils;
using HyoutaUtils.Checksum;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
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
		}

		private static string GetDefaultPathCS1() {
			if (Directory.Exists(SenCommonPaths.Sen1SteamDir)) {
				return SenCommonPaths.Sen1SteamDir;
			}
			if (Directory.Exists(SenCommonPaths.Sen1GalaxyDir)) {
				return SenCommonPaths.Sen1GalaxyDir;
			}
			return @"c:\";
		}

		private static string GetDefaultPathCS2() {
			if (Directory.Exists(SenCommonPaths.Sen2SteamDir)) {
				return SenCommonPaths.Sen2SteamDir;
			}
			if (Directory.Exists(SenCommonPaths.Sen2GalaxyDir)) {
				return SenCommonPaths.Sen2GalaxyDir;
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

		private void OpenCs1GameDir(string sen1patcherpath) {
			// prevent people from being dumb and selecting the wrong game dir
			try {
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(sen1patcherpath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x8dde2b39f128179aul, 0x0beb3301cfd56a98ul, 0xc0f98a55u)) {
						MessageBox.Show("Selected file does not appear to be Sen1Launcher.exe of version 1.6.");
						return;
					}
				}
			} catch (Exception ex) {
				MessageBox.Show("Error while validating Sen1Launcher.exe: " + ex.Message);
				return;
			}

			ProgressReporter progress = new DummyProgressReporter();
			string path = Path.GetDirectoryName(sen1patcherpath);
			if (FilenameFix.FixupIncorrectEncodingInFilenames(path, 1, false, progress)) {
				FilenameFix.FixupIncorrectEncodingInFilenames(path, 1, true, progress);
			}
			var storage = FileModExec.InitializeAndPersistFileStorage(path, Sen1KnownFiles.Files);

			new Sen1Form(path, storage).ShowDialog();
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

		private void OpenCs2GameDir(string sen2patcherpath) {
			// prevent people from being dumb and selecting the wrong game dir
			try {
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(sen2patcherpath)) {
					SHA1 hash = ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					if (hash != new SHA1(0x81024410cc1fd1b4ul, 0x62c600e0378714bdul, 0x7704b202u)) {
						MessageBox.Show("Selected file does not appear to be Sen2Launcher.exe of version 1.4, 1.4.1, or 1.4.2.");
						return;
					}
				}
			} catch (Exception ex) {
				MessageBox.Show("Error while validating Sen2Launcher.exe: " + ex.Message);
				return;
			}

			ProgressReporter progress = new DummyProgressReporter();
			string path = Path.GetDirectoryName(sen2patcherpath);
			if (FilenameFix.FixupIncorrectEncodingInFilenames(path, 2, false, progress)) {
				FilenameFix.FixupIncorrectEncodingInFilenames(path, 2, true, progress);
			}
			var storage = FileModExec.InitializeAndPersistFileStorage(path, Sen2KnownFiles.Files);

			new Sen2Form(path, storage).ShowDialog();
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
	}
}
