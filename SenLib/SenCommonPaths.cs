using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class SenCommonPaths {
		public static readonly string Sen1SteamDir = @"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel\";
		public static readonly string Sen1GalaxyDir = @"c:\Program Files (x86)\GOG Galaxy\Games\The Legend of Heroes - Trails of Cold Steel\";

		public static readonly string Sen2SteamDir = @"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel II\";
		public static readonly string Sen2GalaxyDir = @"c:\Program Files (x86)\GOG Galaxy\Games\The Legend of Heroes Trails of Cold Steel II\";

		private static string _SavedGamesFolder = null;
		public static string SavedGamesFolder {
			get {
				if (_SavedGamesFolder == null) {
					IntPtr str;
					if (SHGetKnownFolderPath(new Guid("4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4"), 0, IntPtr.Zero, out str) == 0) {
						_SavedGamesFolder = Marshal.PtrToStringUni(str);
					}
					if (str != IntPtr.Zero) {
						Marshal.FreeCoTaskMem(str);
					}
				}
				if (_SavedGamesFolder == null) {
					throw new Exception("Failed to get known Saved Games folder.");
				}
				return _SavedGamesFolder;
			}
		}

		public static string Sen1SaveFolder {
			get {
				return Path.Combine(SavedGamesFolder, "FALCOM", "ed8");
			}
		}

		public static string Sen1SystemDataFilename = "save511.dat";

		public static string Sen1SystemDataFile {
			get {
				return Path.Combine(Sen1SaveFolder, Sen1SystemDataFilename);
			}
		}

		public static string Sen2SaveFolder {
			get {
				return Path.Combine(SavedGamesFolder, "FALCOM", "ed8_2");
			}
		}

		public static string Sen2SystemDataFilename = "save255.dat";

		public static string Sen2SystemDataFile {
			get {
				return Path.Combine(Sen2SaveFolder, Sen2SystemDataFilename);
			}
		}

		[DllImport("shell32.dll")]
		static extern int SHGetKnownFolderPath([MarshalAs(UnmanagedType.LPStruct)] Guid rfid, uint dwFlags, IntPtr hToken, out IntPtr ppszPath);
	}
}
