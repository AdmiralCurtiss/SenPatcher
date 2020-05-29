using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public enum SenVersion {
		Sen1_v1_6_En,
		Sen1_v1_6_Jp,
		Sen2_v1_4_1_En,
		Sen2_v1_4_1_Jp,
		Sen2_v1_4_2_En,
		Sen2_v1_4_2_Jp,
	}

	public static class SenVersionIdentifier {
		public static (Stream binary, SenVersion? version) OpenAndIdentifyGame(string path) {
			if (!File.Exists(path)) {
				return (null, null);
			}

			var try1 = TryIdentify(path);

			if (try1.binary != null && try1.version != null) {
				return try1;
			}

			string backuppath = path + SenCommonPaths.BackupPostfix;
			if (File.Exists(backuppath)) {
				// we have a backup file, try that one
				return TryIdentify(backuppath);
			}

			return (null, null);
		}

		private static (Stream binary, SenVersion? version) TryIdentify(string p) {
			using (var s = new FileStream(p, FileMode.Open, FileAccess.Read, FileShare.Read)) {
				if (s.Length > 16 * 1024 * 1024) {
					// way too large to be any CS game
					return (null, null);
				}

				MemoryStream ms = s.CopyToMemory();
				using (SHA1 sha1 = SHA1.Create()) {
					byte[] hashbytes = sha1.ComputeHash(ms);
					ms.Position = 0;
					StringBuilder sb = new StringBuilder(28);
					foreach (byte b in hashbytes) {
						sb.Append(b.ToString("x2"));
					}
					switch (sb.ToString()) {
						case "373c1d1b30001af360042365ed257e070bf40acc": return (ms, SenVersion.Sen1_v1_6_En);
						case "1d56abf5aa02eeae334797c287ef2109c7a103fa": return (ms, SenVersion.Sen1_v1_6_Jp);
						case "d5c333b4cd517d43e3868e159fbec37dba4122d6": return (ms, SenVersion.Sen2_v1_4_1_En);
						case "b8158fb59e43c02e904f813150d841336d1a13e5": return (ms, SenVersion.Sen2_v1_4_1_Jp);
						case "b08ece4ee38e6e3a99e58eb11cffb45e49704f86": return (ms, SenVersion.Sen2_v1_4_2_En);
						case "7d1db7e0bb91ab77a3fd1eba53b0ed25806186c1": return (ms, SenVersion.Sen2_v1_4_2_Jp);
						default: ms.Dispose(); return (null, null);
					}
				}
			}
		}
	}
}
