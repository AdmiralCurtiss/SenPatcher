using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public class Program {
		public static int Main(string[] args) {
			if (args.Length == 1 && args[0] == "__gen_voice_checks") {
				t_voice_tbl.CheckVoiceTable(
					Path.Combine(SenCommonPaths.Sen1SteamDir, "data/text/dat_us/t_voice.tbl"),
					Path.Combine(SenCommonPaths.Sen1SteamDir, "data/voice/wav"),
					Path.Combine(SenCommonPaths.Sen1SteamDir, "voice_check_english.txt")
				);
				t_voice_tbl.CheckVoiceTable(
					Path.Combine(SenCommonPaths.Sen1SteamDir, "data/text/dat/t_voice.tbl"),
					Path.Combine(SenCommonPaths.Sen1SteamDir, "data/voice/wav_jp"),
					Path.Combine(SenCommonPaths.Sen1SteamDir, "voice_check_japanese.txt")
				);
				t_voice_tbl.CheckVoiceTable(
					Path.Combine(SenCommonPaths.Sen2SteamDir, "data/text/dat_us/t_voice.tbl"),
					Path.Combine(SenCommonPaths.Sen2SteamDir, "data/voice/wav"),
					Path.Combine(SenCommonPaths.Sen2SteamDir, "voice_check_english.txt")
				);
				t_voice_tbl.CheckVoiceTable(
					Path.Combine(SenCommonPaths.Sen2SteamDir, "data/text/dat/t_voice.tbl"),
					Path.Combine(SenCommonPaths.Sen2SteamDir, "data/voice_jp/wav"),
					Path.Combine(SenCommonPaths.Sen2SteamDir, "voice_check_japanese.txt")
				);
				return 0;
			}

			if (args.Length == 0) {
				Console.WriteLine("No path to directory given.");
				return -1;
			}

			string path = args[0];
			if (!Directory.Exists(path)) {
				Console.WriteLine($"No directory found at {path}.");
				return -1;
			}

			int sengame;
			if (File.Exists(System.IO.Path.Combine(path, "Sen1Launcher.exe"))) {
				sengame = 1;
			} else if (File.Exists(System.IO.Path.Combine(path, "Sen2Launcher.exe"))) {
				sengame = 2;
			} else {
				Console.WriteLine($"Failed to detect whether {path} is CS1 or 2.");
				return -1;
			}

			FilenameFix.FixupIncorrectEncodingInFilenames(path, sengame);
			FileStorage storage = FileModExec.InitializeAndPersistFileStorage(path, sengame == 1 ? Sen1KnownFiles.Files : Sen2KnownFiles.Files);
			if (storage == null) {
				Console.WriteLine($"Failed to initialize file storage from {path}.");
				return -1;
			}

			PatchResult result;

			if (sengame == 1) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen1Mods.GetExecutableMods(
					removeTurboSkip: true,
					allowR2NotebookShortcut: true,
					turboKey: 0xA,
					fixTextureIds: true,
					correctLanguageVoiceTables: true
				));
				mods.AddRange(Sen1Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods);
			} else {
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					removeTurboSkip: true,
					patchAudioThread: true,
					audioThreadDivisor: 1000,
					patchBgmQueueing: true,
					correctLanguageVoiceTables: true
				));
				mods.AddRange(Sen2Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods);
			}

			if (!result.AllSuccessful) {
				Console.WriteLine($"Failed to patch CS{sengame} at {path}.");
				return -1;
			}

			Console.WriteLine($"Successfully patched CS{sengame} at {path}.");
			return 0;
		}
	}
}
