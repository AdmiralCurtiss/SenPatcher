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
				Console.WriteLine("No directory found at " + path + ".");
				return -1;
			}


			// TODO: detect whether we're Sen1 or 2
			int sengame = 2;

			FileStorage storage = FileModExec.InitializeAndPersistFileStorage(path, sengame == 1 ? Sen1KnownFiles.Files : Sen2KnownFiles.Files);
			if (storage == null) {
				Console.WriteLine("Failed to initialize file storage from " + path + ".");
				return -1;
			}

			PatchResult result;

			if (sengame == 1) {
				result = new Sen1PatchExec(path, storage).ApplyPatches(Sen1PatchExec.GetMods(
					removeTurboSkip: true,
					allowR2NotebookShortcut: true,
					turboKey: 0xA,
					fixTextureIds: true,
					patchAssets: true
				));
			} else {
				result = new Sen2PatchExec(path, storage).ApplyPatches(Sen2PatchExec.GetMods(
					removeTurboSkip: true,
					patchAudioThread: true,
					audioThreadDivisor: 1000,
					patchBgmQueueing: true,
					patchAssets: true
				));
			}

			if (!result.AllSuccessful) {
				Console.WriteLine($"Failed to patch CS${sengame} at ${path}.");
				return -1;
			}

			Console.WriteLine($"Successfully patched CS${sengame} at ${path}.");
			return 0;

			/*
			if (!File.Exists(path)) {
				Console.WriteLine("No file found at " + path + ".");
				return -1;
			}

			Stream binary;
			SenVersion? actualVersion;
			try {
				(binary, actualVersion) = SenVersionIdentifier.OpenAndIdentifyGame(path);
			} catch (Exception ex) {
				Console.WriteLine("Error while identifying " + path + ": " + ex.ToString());
				return -1;
			}

			if (binary == null || actualVersion == null) {
				Console.WriteLine("Could not identify file at " + path + " as any supported Cold Steel executable.");
				return -1;
			}

			switch (actualVersion) {
				case SenVersion.Sen1_v1_6_Jp:
				case SenVersion.Sen1_v1_6_En: {
					bool success = new Sen1PatchExec(path, binary, actualVersion.Value).ApplyPatches(
						removeTurboSkip: true,
						allowR2NotebookShortcut: true,
						turboKey: 0xA,
						fixTextureIds: true,
						patchAssets: true
					).AllSuccessful;

					if (success) {
						Console.WriteLine("Successfully patched CS1 at " + path + ".");
					} else {
						Console.WriteLine("Failed to patch CS1 at " + path + ".");
						return -1;
					}
					break;
				}
				case SenVersion.Sen2_v1_4_1_Jp:
				case SenVersion.Sen2_v1_4_1_En:
				case SenVersion.Sen2_v1_4_2_Jp:
				case SenVersion.Sen2_v1_4_2_En: {
					bool success = new Sen2PatchExec(path, binary, actualVersion.Value).ApplyPatches(
						removeTurboSkip: true,
						patchAudioThread: true,
						audioThreadDivisor: 1000,
						patchBgmQueueing: true,
						patchAssets: true
					).AllSuccessful;

					if (success) {
						Console.WriteLine("Successfully patched CS2 at " + path + ".");
					} else {
						Console.WriteLine("Failed to patch CS2 at " + path + ".");
						return -1;
					}
					break;
				}
				default:
					Console.WriteLine("File at " + path + " was recognized but is not supported.");
					return -1;
			}

			return 0;
			*/
		}
	}
}
