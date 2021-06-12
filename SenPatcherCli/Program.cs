using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
using SenLib.Sen3;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public class Program {
		public static int Main(string[] args) {
			if (args.Length >= 2 && args[0] == "--parse-script") {
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(args[1])) {
					var funcs = ScriptParser.Parse(fs.CopyToByteArrayStreamAndDispose(), false);

					using (var outfs = new FileStream(args.Length > 2 ? args[2] : args[1] + ".txt", FileMode.Create)) {
						foreach (var func in funcs) {
							outfs.WriteUTF8(func.Name);
							outfs.WriteUTF8("\n");
							outfs.WriteUTF8("\n");
							foreach (var op in func.Ops) {
								outfs.WriteUTF8(op);
								outfs.WriteUTF8("\n");
							}
							outfs.WriteUTF8("\n");
							outfs.WriteUTF8("\n");
						}
					}
				}
				return 0;
			}

			if (args.Length >= 2 && args[0] == "--parse-book") {
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(args[1])) {
					var raw = fs.CopyToByteArrayStreamAndDispose();
					var book = new BookTable(raw.Duplicate());
					var reserialized_le = book.WriteToStream(EndianUtils.Endianness.LittleEndian);
					var reserialized_be = book.WriteToStream(EndianUtils.Endianness.BigEndian);
					if (!(StreamUtils.IsIdentical(raw, reserialized_le) || StreamUtils.IsIdentical(raw, reserialized_be))) {
						Console.WriteLine("Does not re-serialize correctly. Maybe we have an error in the data structure?");
					}

					using (var outfs = new FileStream(args.Length > 2 ? args[2] : args[1] + ".txt", FileMode.Create)) {
						outfs.WriteUTF8(string.Format("{0}\n\n", book.Name));
						foreach (var entry in book.Entries) {
							outfs.WriteUTF8(string.Format(" ====== {0} ======= \n", entry.Name));
							if (entry.Book99_Value1 != null) {
								outfs.WriteUTF8(string.Format("99_1: {0}\n", entry.Book99_Value1.Value));
							}
							if (entry.Book99_Value2 != null) {
								outfs.WriteUTF8(string.Format("99_2: {0}\n", entry.Book99_Value2.Value));
							}
							if (entry.BookDataStructs != null) {
								foreach (var ds in entry.BookDataStructs) {
									outfs.WriteUTF8(string.Format("Value_1: {0}\n", ds.Unknown1));
									outfs.WriteUTF8(string.Format("String: {0}\n", Encoding.ASCII.GetString(ds.Name).TrimNull()));
									outfs.WriteUTF8(string.Format("Value_2: {0}\n", ds.Unknown2));
									outfs.WriteUTF8(string.Format("Value_3: {0}\n", ds.Unknown3));
									outfs.WriteUTF8(string.Format("Value_4: {0}\n", ds.Unknown4));
									outfs.WriteUTF8(string.Format("Value_5: {0}\n", ds.Unknown5));
									outfs.WriteUTF8(string.Format("Value_6: {0}\n", ds.Unknown6));
									outfs.WriteUTF8(string.Format("Value_7: {0}\n", ds.Unknown7));
									outfs.WriteUTF8(string.Format("Value_8: {0}\n", ds.Unknown8));
									outfs.WriteUTF8(string.Format("Value_9: {0}\n", ds.Unknown9));
									outfs.WriteUTF8(string.Format("Value_10: {0}\n", ds.Unknown10));
									outfs.WriteUTF8(string.Format("Value_11: {0}\n", ds.Unknown11));
								}
							}
							if (entry.Text != null) {
								foreach (var t in entry.Text.Split(new string[] { "\\n" }, StringSplitOptions.None)) {
									outfs.WriteUTF8(t);
									outfs.WriteUTF8("\n");
								}
							}
							outfs.WriteUTF8("\n");
						}
					}
				}
				return 0;
			}

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
			} else if (File.Exists(System.IO.Path.Combine(path, "Sen3Launcher.exe"))) {
				sengame = 3;
			} else {
				Console.WriteLine($"Failed to detect what game {path} is.");
				return -1;
			}

			FilenameFix.FixupIncorrectEncodingInFilenames(path, sengame, true, new CliProgressReporter());
			KnownFile[] knownFiles;
			switch (sengame) {
				case 1: knownFiles = Sen1KnownFiles.Files; break;
				case 2: knownFiles = Sen2KnownFiles.Files; break;
				case 3: knownFiles = Sen3KnownFiles.Files; break;
				default: return -1; // shouldn't get here
			}
			FileStorage storage = FileModExec.InitializeAndPersistFileStorage(path, knownFiles, new CliProgressReporter())?.Storage;
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
					correctLanguageVoiceTables: true,
					disableMouseCapture: true,
					disablePauseOnFocusLoss: true
				));
				mods.AddRange(Sen1Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else if (sengame == 2) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					removeTurboSkip: true,
					patchAudioThread: true,
					audioThreadDivisor: 1000,
					patchBgmQueueing: true,
					correctLanguageVoiceTables: true,
					disableMouseCapture: true,
					disablePauseOnFocusLoss: true
				));
				mods.AddRange(Sen2Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else if (sengame == 3) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen3Mods.GetExecutableMods(
					fixInGameButtonMappingValidity: true,
					allowSwitchToNightmare: true
				));
				mods.AddRange(Sen3Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else {
				return -1; // shouldn't get here
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
