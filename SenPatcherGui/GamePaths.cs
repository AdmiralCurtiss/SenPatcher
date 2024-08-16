using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherGui {
	internal static class GamePaths {
		private static string FindExistingPath(
			string configuredPath,
			string[] otherGamesConfiguredPaths,
			string[] foldersToCheck,
			string filenameToCheck
		) {
			// first check the configured path itself
			try {
				if (configuredPath != "") {
					if (File.Exists(Path.Combine(configuredPath, filenameToCheck))) {
						return configuredPath;
					}
				}
			} catch (Exception) { }

			// then check if we can use a different configured path to find the target one
			foreach (string p in otherGamesConfiguredPaths) {
				try {
					if (p != "") {
						string pp = Directory.GetParent(p)?.FullName;
						if (pp != null) {
							foreach (string f in foldersToCheck) {
								try {
									if (File.Exists(Path.Combine(pp, f, filenameToCheck))) {
										return Path.Combine(pp, f);
									}
								} catch (Exception) { }
							}
						}
					}
				} catch (Exception) { }
			}

			// didn't find anything, check for plausible default paths
			//bool isWindows = System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(System.Runtime.InteropServices.OSPlatform.Windows);
			bool isWindows = true;
			if (isWindows) {
				try {
					foreach (var drive in DriveInfo.GetDrives()) {
						foreach (string f in foldersToCheck) {
							try {
								string path = Path.Combine(drive.Name, @"Program Files (x86)\Steam\steamapps\common", f);
								if (File.Exists(Path.Combine(path, filenameToCheck))) {
									return path;
								}
							} catch (Exception) { }
							try {
								string path = Path.Combine(drive.Name, @"SteamLibrary\steamapps\common", f);
								if (File.Exists(Path.Combine(path, filenameToCheck))) {
									return path;
								}
							} catch (Exception) { }
							try {
								string path = Path.Combine(drive.Name, @"Program Files (x86)\GOG Galaxy\Games", f);
								if (File.Exists(Path.Combine(path, filenameToCheck))) {
									return path;
								}
							} catch (Exception) { }
						}
					}
				} catch (Exception) { }
			}

			string prefix = isWindows ? "Z:" : "";
			try {
				foreach (string f in foldersToCheck) {
					try {
						string path = Path.Combine(prefix + "/home/deck/.local/share/Steam/steamapps/common", f);
						if (File.Exists(Path.Combine(path, filenameToCheck))) {
							return path;
						}
					} catch (Exception) { }
					try {
						string path = Path.Combine(prefix + "/home/" + Environment.UserName + "/.local/share/Steam/steamapps/common", f);
						if (File.Exists(Path.Combine(path, filenameToCheck))) {
							return path;
						}
					} catch (Exception) { }
					try {
						string path = Path.Combine(prefix + "/home/" + Environment.UserName + "/.steam/root/steamapps/common", f);
						if (File.Exists(Path.Combine(path, filenameToCheck))) {
							return path;
						}
					} catch (Exception) { }
				}
				foreach (string mountroot in new string[] { "/run/media", "/media", "/mnt" }) {
					try {
						foreach (string dir in Directory.EnumerateDirectories(prefix + mountroot)) {
							foreach (string f in foldersToCheck) {
								try {
									string path = Path.Combine(dir, "steamapps/common", f);
									if (File.Exists(Path.Combine(path, filenameToCheck))) {
										return path;
									}
								} catch (Exception) { }
							}
						}
					} catch (Exception) { }
				}
			} catch (Exception) { }

			return null;
		}

		public static string GetDefaultPathCS1() {
			var p = FindExistingPath(
				Properties.Settings.Default.Sen1Path,
				new string[] {
					Properties.Settings.Default.Sen2Path,
					Properties.Settings.Default.Sen3Path,
					Properties.Settings.Default.Sen4Path,
					Properties.Settings.Default.Sen5Path,
					Properties.Settings.Default.TXPath,
				},
				new string[] {
					"Trails of Cold Steel",
					"The Legend of Heroes - Trails of Cold Steel",
				},
				"Sen1Launcher.exe"
			);
			return p ?? "";
		}

		public static string GetDefaultPathCS2() {
			var p = FindExistingPath(
				Properties.Settings.Default.Sen2Path,
				new string[] {
					Properties.Settings.Default.Sen1Path,
					Properties.Settings.Default.Sen3Path,
					Properties.Settings.Default.Sen4Path,
					Properties.Settings.Default.Sen5Path,
					Properties.Settings.Default.TXPath,
				},
				new string[] {
					"Trails of Cold Steel II",
					"The Legend of Heroes Trails of Cold Steel II",
				},
				"Sen2Launcher.exe"
			);
			return p ?? "";
		}

		public static string GetDefaultPathCS3() {
			var p = FindExistingPath(
				Properties.Settings.Default.Sen3Path,
				new string[] {
					Properties.Settings.Default.Sen1Path,
					Properties.Settings.Default.Sen2Path,
					Properties.Settings.Default.Sen4Path,
					Properties.Settings.Default.Sen5Path,
					Properties.Settings.Default.TXPath,
				},
				new string[] {
					"The Legend of Heroes Trails of Cold Steel III",
				},
				"Sen3Launcher.exe"
			);
			return p ?? "";
		}

		public static string GetDefaultPathCS4() {
			var p = FindExistingPath(
				Properties.Settings.Default.Sen4Path,
				new string[] {
					Properties.Settings.Default.Sen1Path,
					Properties.Settings.Default.Sen2Path,
					Properties.Settings.Default.Sen3Path,
					Properties.Settings.Default.Sen5Path,
					Properties.Settings.Default.TXPath,
				},
				new string[] {
					"The Legend of Heroes Trails of Cold Steel IV",
				},
				"Sen4Launcher.exe"
			);
			return p ?? "";
		}

		public static string GetDefaultPathReverie() {
			var p = FindExistingPath(
				Properties.Settings.Default.Sen5Path,
				new string[] {
					Properties.Settings.Default.Sen1Path,
					Properties.Settings.Default.Sen2Path,
					Properties.Settings.Default.Sen3Path,
					Properties.Settings.Default.Sen4Path,
					Properties.Settings.Default.TXPath,
				},
				new string[] {
					"The Legend of Heroes Trails into Reverie",
				},
				"bin/Win64/hnk.exe"
			);
			return p ?? "";
		}

		public static string GetDefaultPathTX() {
			var p = FindExistingPath(
				Properties.Settings.Default.TXPath,
				new string[] {
					Properties.Settings.Default.Sen1Path,
					Properties.Settings.Default.Sen2Path,
					Properties.Settings.Default.Sen3Path,
					Properties.Settings.Default.Sen4Path,
					Properties.Settings.Default.Sen5Path,
				},
				new string[] {
					"Tokyo Xanadu eX+",
				},
				"TokyoXanadu.exe"
			);
			return p ?? "";
		}

	}
}
