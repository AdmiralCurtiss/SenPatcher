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
		Sen1_v1_0_En,
		Sen1_v1_0_Jp,
		Sen1_v1_1_En,
		Sen1_v1_1_Jp,
		Sen1_v1_2_1_En,
		Sen1_v1_2_1_Jp,
		Sen1_v1_3_En,
		Sen1_v1_3_Jp,
		Sen1_v1_3_5_En,
		Sen1_v1_3_5_Jp,
		Sen1_v1_4_En,
		Sen1_v1_4_Jp,
		Sen1_v1_5_En,
		Sen1_v1_5_Jp,
		Sen1_v1_6_En,
		Sen1_v1_6_Jp,

		Sen1Launcher_v1_0,
		Sen1Launcher_v1_1,
		Sen1Launcher_v1_2_1,
		Sen1Launcher_v1_3_to_v1_5,
		Sen1Launcher_v1_6,

		Sen2_v1_0_En,
		Sen2_v1_0_Jp,
		Sen2_v1_1_En,
		Sen2_v1_1_Jp,
		Sen2_v1_2_En,
		Sen2_v1_2_Jp,
		Sen2_v1_3_En,
		Sen2_v1_3_Jp,
		Sen2_v1_3_1_En,
		Sen2_v1_3_1_Jp,
		Sen2_v1_4_En,
		Sen2_v1_4_Jp,
		Sen2_v1_4_1_En,
		Sen2_v1_4_1_Jp,
		Sen2_v1_4_2_En,
		Sen2_v1_4_2_Jp,

		Sen2Launcher_v1_0,
		Sen2Launcher_v1_1,
		Sen2Launcher_v1_2_to_v1_3_1,
		Sen2Launcher_v1_4_to_v1_4_2,
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
						case "1b924e9d27db9bcf292854ffc9fb0ce756398b15": return (ms, SenVersion.Sen1_v1_0_En);
						case "0f0c453a0b94b38d72f2a21706e2d8f98fb4c4c1": return (ms, SenVersion.Sen1_v1_0_Jp);
						case "aa232c99600f66de7141783bef9545ba1c29b5c2": return (ms, SenVersion.Sen1Launcher_v1_0);
						case "b8aa8c6f6e3b4d93cd1aaa4c538a7183aee18145": return (ms, SenVersion.Sen1_v1_1_En);
						case "d6eeef15f2bf32dd69c31eb9ecb3cb4efd310569": return (ms, SenVersion.Sen1_v1_1_Jp);
						case "481c7ef8cdbfad4018a3a86994d5503385f021a2": return (ms, SenVersion.Sen1Launcher_v1_1);
						case "4ff1314b93cc723f72593f102e079ba6d73ea84a": return (ms, SenVersion.Sen1_v1_2_1_En);
						case "99406c618ee8cde46601fcc054e991a5909e893b": return (ms, SenVersion.Sen1_v1_2_1_Jp);
						case "61c232a4f094f44f17f5f6a292e87b2d754e6fb8": return (ms, SenVersion.Sen1Launcher_v1_2_1);
						case "0b58ab4ac3f9b1a72c8620761967e27a75b27b87": return (ms, SenVersion.Sen1_v1_3_En);
						case "7c81b59843c77df7d0383aee084711552c604843": return (ms, SenVersion.Sen1_v1_3_Jp);
						case "d4b0f2d8342072dab9d056c5a5ca55e8b57c9413": return (ms, SenVersion.Sen1Launcher_v1_3_to_v1_5);
						case "5863913f54094139455427a7e11e47891a6b962d": return (ms, SenVersion.Sen1_v1_3_5_En);
						case "337408d92899ca7578f4415fe357ec5a9a648aba": return (ms, SenVersion.Sen1_v1_3_5_Jp);
						case "a3eded914bfe1709e120eee88958e442ae5724f5": return (ms, SenVersion.Sen1_v1_4_En);
						case "8fd6791a20cb51aceb6d6563922726f5e09816fc": return (ms, SenVersion.Sen1_v1_4_Jp);
						case "3f36c6fb023b4d8247c0087472c0c7c13e967331": return (ms, SenVersion.Sen1_v1_5_En);
						case "0cb40345427f5e36001524534e0a6af0c59d74b7": return (ms, SenVersion.Sen1_v1_5_Jp);
						case "373c1d1b30001af360042365ed257e070bf40acc": return (ms, SenVersion.Sen1_v1_6_En);
						case "1d56abf5aa02eeae334797c287ef2109c7a103fa": return (ms, SenVersion.Sen1_v1_6_Jp);
						case "8dde2b39f128179a0beb3301cfd56a98c0f98a55": return (ms, SenVersion.Sen1Launcher_v1_6);

						case "f30bdd323ee04e5ddf663d3e6a31437ab1bdfb67": return (ms, SenVersion.Sen2_v1_0_Jp);
						case "a349d1d3d9a63bc7b950907fbc496a4bb9e1410d": return (ms, SenVersion.Sen2_v1_0_En);
						case "49ebc1dbc732c61ec9fbee2c5ea7de78b3a22afc": return (ms, SenVersion.Sen2Launcher_v1_0);
						case "2ab4484a8b6b86e94b618a5acaa68a556009d777": return (ms, SenVersion.Sen2_v1_1_Jp);
						case "de64bc4e94ed0a9b5dc8677590292381a29f4cd6": return (ms, SenVersion.Sen2_v1_1_En);
						case "afcd9ce356e90534a8a2774e06dede8d356b9c14": return (ms, SenVersion.Sen2Launcher_v1_1);
						case "f9918032f8fcdd4edcb479e3255321f801f932d1": return (ms, SenVersion.Sen2_v1_2_Jp);
						case "5855560abecd00ec2c8efcbee02444ca1738823c": return (ms, SenVersion.Sen2_v1_2_En);
						case "b6f27984e55e4569d51584b8dad42fc8d6198efe": return (ms, SenVersion.Sen2Launcher_v1_2_to_v1_3_1);
						case "335fb5740bd3eb8703a12f935b30b01baf354a0c": return (ms, SenVersion.Sen2_v1_3_Jp);
						case "0a507b9e592dce1f77fbe9f4956781c40db1a5cc": return (ms, SenVersion.Sen2_v1_3_En);
						case "a4e834f2c21fb21bb3c981def0c8363abe8538d3": return (ms, SenVersion.Sen2_v1_3_1_Jp);
						case "5eeebf4edb09df316a8869e0e749de9cd61a6e88": return (ms, SenVersion.Sen2_v1_3_1_En);
						case "825e2643338963565f49e3c40aa0aec1d77229fa": return (ms, SenVersion.Sen2_v1_4_Jp);
						case "e5f2e2682557af7a2f52b2299ba0980f218c5e66": return (ms, SenVersion.Sen2_v1_4_En);
						case "81024410cc1fd1b462c600e0378714bd7704b202": return (ms, SenVersion.Sen2Launcher_v1_4_to_v1_4_2);
						case "b8158fb59e43c02e904f813150d841336d1a13e5": return (ms, SenVersion.Sen2_v1_4_1_Jp);
						case "d5c333b4cd517d43e3868e159fbec37dba4122d6": return (ms, SenVersion.Sen2_v1_4_1_En);
						case "7d1db7e0bb91ab77a3fd1eba53b0ed25806186c1": return (ms, SenVersion.Sen2_v1_4_2_Jp);
						case "b08ece4ee38e6e3a99e58eb11cffb45e49704f86": return (ms, SenVersion.Sen2_v1_4_2_En);

						default: ms.Dispose(); return (null, null);
					}
				}
			}
		}
	}
}
