using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class SenCommonPaths {
		public static readonly string Sen1SteamDir = @"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel\";
		public static readonly string Sen1GalaxyDir = @"c:\Program Files (x86)\GOG Galaxy\Games\The Legend of Heroes - Trails of Cold Steel\";
		public static readonly string Sen1EnExePath = "ed8.exe";
		public static readonly string Sen1JpExePath = "ed8jp.exe";

		public static readonly string Sen2SteamDir = @"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel II\";
		public static readonly string Sen2GalaxyDir = @"c:\Program Files (x86)\GOG Galaxy\Games\The Legend of Heroes Trails of Cold Steel II\";
		public static readonly string Sen2EnExePath = @"bin\Win32\ed8_2_PC_US.exe";
		public static readonly string Sen2JpExePath = @"bin\Win32\ed8_2_PC_JP.exe";

		public static readonly string BackupPostfix = ".senpatcher.bkp";
	}
}
