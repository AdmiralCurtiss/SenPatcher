using HyoutaPluginBase;
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

			// it's possible that someone is launching this on CS2 1.4, because that's the last build that was pushed to Humble Bundle
			// if so, offer an update to 1.4.2
			try {
				if (!TryPatchCs2Version14(path)) {
					return null;
				}
			} catch (Exception ex) {
				ShowError("Error while checking for CS2 1.4: " + ex.Message);
				return null;
			}

			return path;
		}

		private void TryMoveTempFileIfMatches(string path, string tmppath, SHA1 hash) {
			if (File.Exists(tmppath)) {
				var filedata = new HyoutaUtils.Streams.DuplicatableFileStream(tmppath).CopyToByteArrayStreamAndDispose();
				if (hash == ChecksumUtils.CalculateSHA1ForEntireStream(filedata)) {
					SenUtils.TryDeleteFile(path);
					File.Move(tmppath, path);
				}
			}
		}

		private bool TryPatchCs2Version14(string path) {
			string path_exe_jp = Path.Combine(path, "bin/Win32/ed8_2_PC_JP.exe");
			string path_exe_us = Path.Combine(path, "bin/Win32/ed8_2_PC_US.exe");
			string path_t1001_jp = Path.Combine(path, "data/scripts/scena/dat/t1001.dat");
			string path_t1001_us = Path.Combine(path, "data/scripts/scena/dat_us/t1001.dat");
			string path_asm_folder = Path.Combine(path, "data/scripts/scena/asm");
			string path_asm_file = Path.Combine(path, "data/scripts/scena/asm/t1001.tbl");
			string path_exe_jp_tmp = path_exe_jp + ".cs142";
			string path_exe_us_tmp = path_exe_us + ".cs142";
			string path_t1001_jp_tmp = path_t1001_jp + ".cs142";
			string path_t1001_us_tmp = path_t1001_us + ".cs142";
			string path_asm_file_tmp = path_asm_file + ".cs142";
			SHA1 jp142hash = new SHA1(0x7d1db7e0bb91ab77ul, 0xa3fd1eba53b0ed25ul, 0x806186c1u);
			SHA1 us142hash = new SHA1(0xb08ece4ee38e6e3aul, 0x99e58eb11cffb45eul, 0x49704f86u);
			SHA1 t1001jp142hash = new SHA1(0x24b90bc222efb431ul, 0xa05941973b3bcbd7ul, 0xe3599d81u);
			SHA1 t1001us142hash = new SHA1(0xfae1d23cd07aa0c9ul, 0x90ca63607e64fcddul, 0xd60a80dau);
			SHA1 t1001asm142hash = new SHA1(0x568a1ae375a6077eul, 0xf5c6fb8e277a333ful, 0x1979505bu);
			SHA1 us14hash = new SHA1(0xe5f2e2682557af7aul, 0x2f52b2299ba0980ful, 0x218c5e66u);
			SHA1 jp14hash = new SHA1(0x825e264333896356ul, 0x5f49e3c40aa0aec1ul, 0xd77229fau);
			SHA1 t1001us14hash = new SHA1(0xace845b437df94fbul, 0xfe2d638a2ec162b4ul, 0x92a657b3u);
			SHA1 t1001jp14hash = new SHA1(0x3d75d79e3201f8f5ul, 0xac61c206f8cc86dbul, 0x7c4651ddu);

			// try to recover a patch that was interrupted during the writeout phase
			TryMoveTempFileIfMatches(path_exe_jp, path_exe_jp_tmp, jp142hash);
			TryMoveTempFileIfMatches(path_exe_us, path_exe_us_tmp, us142hash);
			TryMoveTempFileIfMatches(path_t1001_jp, path_t1001_jp_tmp, t1001jp142hash);
			TryMoveTempFileIfMatches(path_t1001_us, path_t1001_us_tmp, t1001us142hash);
			TryMoveTempFileIfMatches(path_asm_file, path_asm_file_tmp, t1001asm142hash);
			SenUtils.TryDeleteFile(path_exe_jp_tmp);
			SenUtils.TryDeleteFile(path_exe_us_tmp);
			SenUtils.TryDeleteFile(path_t1001_jp_tmp);
			SenUtils.TryDeleteFile(path_t1001_us_tmp);
			SenUtils.TryDeleteFile(path_asm_file_tmp);

			if (File.Exists(path_asm_file)) {
				return true;
			}
			var exe_us_14 = new HyoutaUtils.Streams.DuplicatableFileStream(path_exe_us).CopyToByteArrayStreamAndDispose();
			if (us14hash != ChecksumUtils.CalculateSHA1ForEntireStream(exe_us_14)) {
				return true;
			}
			var exe_jp_14 = new HyoutaUtils.Streams.DuplicatableFileStream(path_exe_jp).CopyToByteArrayStreamAndDispose();
			if (jp14hash != ChecksumUtils.CalculateSHA1ForEntireStream(exe_jp_14)) {
				return true;
			}
			var t1001_us_14 = new HyoutaUtils.Streams.DuplicatableFileStream(path_t1001_us).CopyToByteArrayStreamAndDispose();
			if (t1001us14hash != ChecksumUtils.CalculateSHA1ForEntireStream(t1001_us_14)) {
				return true;
			}
			var t1001_jp_14 = new HyoutaUtils.Streams.DuplicatableFileStream(path_t1001_jp).CopyToByteArrayStreamAndDispose();
			if (t1001jp14hash != ChecksumUtils.CalculateSHA1ForEntireStream(t1001_jp_14)) {
				return true;
			}

			// it's sloppy to mix in GUI code like this but whatever, this is not very reusable anyway...
			Logging.Log("CS2 1.4 found.");
			var result = MessageBox.Show("This appears to be version 1.4 of CS2.\n" +
				"SenPatcher does not support this version directly, but it can update the game to version 1.4.2, which is supported.\n\n" +
				"Would you like to perform this update?",
				"Old CS2 version found", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
			if (result != DialogResult.Yes) {
				Logging.Log("Declined CS2 update.");
				return false;
			}

			Logging.Log("Performing CS2 update.");

			var jp140exe = new SectionSplitExe(exe_jp_14);
			var jp142header = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.header.Duplicate(), new HyoutaUtils.Streams.DuplicatableByteArrayStream(SenLib.Properties.Resources.ed82_j140_to_j142_header)).CopyToByteArrayStreamAndDispose();
			var jp142text = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.text.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j140_to_j142_text).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var jp142rdata = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.rdata.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j140_to_j142_rdata).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var jp142data = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.data.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j140_to_j142_data).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var jp142rsrc = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp140exe.rsrc.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j140_to_j142_rsrc).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var jp142 = HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { jp142header.Duplicate(), jp142text.Duplicate(), jp142rdata.Duplicate(), jp142data.Duplicate(), jp142rsrc.Duplicate() }).CopyToByteArrayStreamAndDispose();
			if (ChecksumUtils.CalculateSHA1ForEntireStream(jp142) == jp142hash) {
				Logging.Log(string.Format("Acquired {0} (CS2 JP 1.4.2) from 1.4.0!", jp142hash.ToString()));
			} else {
				// very unlikely
				Logging.Log("CS2 JP 1.4.2 patch did not apply correctly.");
				return false;
			}
			var us140exe = new SectionSplitExe(exe_us_14);
			var us142header = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142header.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j142_to_u142_header).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var us142text = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142text.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j142_to_u142_text).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var us142rdata = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { us140exe.rdata.Duplicate(), jp142rdata.Duplicate() }), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_u140j142_to_u142_rdata).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var us142data = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142data.Duplicate(), DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.ed82_j142_to_u142_data).CopyToByteArrayStreamAndDispose()).CopyToByteArrayStreamAndDispose();
			var us142rsrc = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(jp142rsrc.Duplicate(), new HyoutaUtils.Streams.DuplicatableByteArrayStream(SenLib.Properties.Resources.ed82_j142_to_u142_rsrc)).CopyToByteArrayStreamAndDispose();
			var us142 = HyoutaUtils.Streams.ConcatenatedStream.CreateConcatenatedStream(new List<DuplicatableStream>() { us142header.Duplicate(), us142text.Duplicate(), us142rdata.Duplicate(), us142data.Duplicate(), us142rsrc.Duplicate() }).CopyToByteArrayStreamAndDispose();
			if (ChecksumUtils.CalculateSHA1ForEntireStream(us142) == us142hash) {
				SenLib.Logging.Log(string.Format("Acquired {0} (CS2 US 1.4.2) from 1.4.0!", us142hash.ToString()));
			} else {
				// very unlikely
				Logging.Log("CS2 US 1.4.2 patch did not apply correctly.");
				return false;
			}

			var t1001_jp_142 = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(t1001_jp_14, new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x51, 0x59, 0x89, 0x51, 0x59, 0x89, 0x80, 0x50, 0x13, 0x93, 0x81, 0xfe, 0x68, 0x54, 0x95, 0xd1, 0x34, 0x92, 0x1c, 0x08, 0x78, 0xd2, 0xa8, 0x99, 0xe0, 0x0c, 0x42 }));
			if (ChecksumUtils.CalculateSHA1ForEntireStream(t1001_jp_142) == t1001jp142hash) {
				Logging.Log(string.Format("Acquired {0} (t1001 JP 1.4.2) from 1.4.0!", t1001jp142hash.ToString()));
			} else {
				// very unlikely
				Logging.Log("t1001 JP 1.4.2 patch did not apply correctly.");
				return false;
			}
			var t1001_us_142 = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(t1001_us_14, new HyoutaUtils.Streams.DuplicatableByteArrayStream(new byte[] { 0x42, 0x50, 0x53, 0x31, 0x11, 0x6a, 0x8a, 0x11, 0x6a, 0x8a, 0x80, 0x28, 0x76, 0x94, 0x81, 0xfe, 0x10, 0x34, 0x98, 0x4b, 0x18, 0x20, 0x67, 0x25, 0xf8, 0x4d, 0x9d, 0xc0, 0x33, 0x34, 0xcd }));
			if (ChecksumUtils.CalculateSHA1ForEntireStream(t1001_us_142) == t1001us142hash) {
				Logging.Log(string.Format("Acquired {0} (t1001 US 1.4.2) from 1.4.0!", t1001us142hash.ToString()));
			} else {
				// very unlikely
				Logging.Log("t1001 US 1.4.2 patch did not apply correctly.");
				return false;
			}
			var t1001_asm_142 = HyoutaUtils.Bps.BpsPatcher.ApplyPatchToStream(t1001_jp_142, DecompressHelper.DecompressFromBuffer(SenLib.Properties.Resources.t1001_dat_to_tbl).CopyToByteArrayStreamAndDispose());
			if (ChecksumUtils.CalculateSHA1ForEntireStream(t1001_asm_142) == t1001asm142hash) {
				Logging.Log(string.Format("Acquired {0} (t1001 asm 1.4.2) from 1.4.0!", t1001asm142hash.ToString()));
			} else {
				// very unlikely
				Logging.Log("t1001 asm 1.4.2 patch did not apply correctly.");
				return false;
			}

			// patching success, write out
			Directory.CreateDirectory(path_asm_folder);
			File.WriteAllBytes(path_exe_jp_tmp + ".tmp", jp142.CopyToByteArrayAndDispose());
			File.WriteAllBytes(path_exe_us_tmp + ".tmp", us142.CopyToByteArrayAndDispose());
			File.WriteAllBytes(path_t1001_jp_tmp + ".tmp", t1001_jp_142.CopyToByteArrayAndDispose());
			File.WriteAllBytes(path_t1001_us_tmp + ".tmp", t1001_us_142.CopyToByteArrayAndDispose());
			File.WriteAllBytes(path_asm_file_tmp + ".tmp", t1001_asm_142.CopyToByteArrayAndDispose());
			File.Move(path_exe_us_tmp + ".tmp", path_exe_us_tmp);
			File.Move(path_exe_jp_tmp + ".tmp", path_exe_jp_tmp);
			File.Move(path_t1001_jp_tmp + ".tmp", path_t1001_jp_tmp);
			File.Move(path_t1001_us_tmp + ".tmp", path_t1001_us_tmp);
			File.Move(path_asm_file_tmp + ".tmp", path_asm_file_tmp);
			SenUtils.TryDeleteFile(path_exe_us);
			File.Move(path_exe_us_tmp, path_exe_us);
			SenUtils.TryDeleteFile(path_exe_jp);
			File.Move(path_exe_jp_tmp, path_exe_jp);
			SenUtils.TryDeleteFile(path_t1001_jp);
			File.Move(path_t1001_jp_tmp, path_t1001_jp);
			SenUtils.TryDeleteFile(path_t1001_us);
			File.Move(path_t1001_us_tmp, path_t1001_us);
			SenUtils.TryDeleteFile(path_asm_file);
			File.Move(path_asm_file_tmp, path_asm_file);

			return true;
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

	internal class SectionSplitExe {
		public DuplicatableStream header;
		public DuplicatableStream text;
		public DuplicatableStream rdata;
		public DuplicatableStream data;
		public DuplicatableStream rsrc;

		public SectionSplitExe(DuplicatableStream s) {
			var exe = new PeExe(s, EndianUtils.Endianness.LittleEndian);
			header = new HyoutaUtils.Streams.PartialStream(s, 0, exe.SectionHeaders[0].PointerToRawData);
			text = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[0].PointerToRawData, exe.SectionHeaders[0].SizeOfRawData);
			rdata = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[1].PointerToRawData, exe.SectionHeaders[1].SizeOfRawData);
			data = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[2].PointerToRawData, exe.SectionHeaders[2].SizeOfRawData);
			rsrc = new HyoutaUtils.Streams.PartialStream(s, exe.SectionHeaders[3].PointerToRawData, exe.SectionHeaders[3].SizeOfRawData);
		}
	}
}
