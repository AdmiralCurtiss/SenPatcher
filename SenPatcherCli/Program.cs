using HyoutaUtils;
using SenLib;
using SenLib.Sen1;
using SenLib.Sen2;
using SenLib.Sen3;
using SenLib.Sen4;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public class Program {
		public static int Main(string[] args) {
			//Playground.Run();
			//return 0;

			int sengame;

			if (args.Length >= 2 && args[0] == "--extract-pkg") {
				string outpath = args.Length >= 3 ? args[2] : args[1] + ".ex";
				Directory.CreateDirectory(outpath);
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(args[1]))
				using (var pkg = new Pkg(fs)) {
					foreach (PkgFile file in pkg.Files) {
						Console.WriteLine("Flags 0x{0:x2} for {1}", file.Flags, file.Filename);
						using (var s = file.DataStream)
						using (var outfs = new FileStream(Path.Combine(outpath, file.Filename), FileMode.Create)) {
							StreamUtils.CopyStream(s, outfs);
						}
					}
				}
				return 0;
			}

			if (args.Length >= 2 && args[0] == "--extract-pka-to-pkg") {
				string outpath = args.Length >= 3 ? args[2] : args[1] + ".ex";
				Directory.CreateDirectory(outpath);
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(args[1]))
				using (var pka = new Pka(fs)) {
					for (int i = 0; i < pka.PkgCount; ++i) {
						string pkgName = pka.GetPkgName(i);
						Console.WriteLine("Building {0}", pkgName);
						using (var pkgStream = pka.BuildPkgToMemory(i))
						using (var outfs = new FileStream(Path.Combine(outpath, pkgName), FileMode.Create)) {
							StreamUtils.CopyStream(pkgStream, outfs);
						}
					}
				}
				return 0;
			}

			if (args.Length >= 2 && args[0] == "--parse-script") {
				string inputfilename = args[1];
				string outputfilename = inputfilename + ".txt";
				string voicetablefilename = args[2];
				sengame = int.Parse(args[3]);
				EndianUtils.Endianness endian = args[4] == "be" ? EndianUtils.Endianness.BigEndian : EndianUtils.Endianness.LittleEndian;
				TextUtils.GameTextEncoding encoding = args[5] == "sjis" ? TextUtils.GameTextEncoding.ShiftJIS : TextUtils.GameTextEncoding.UTF8;
				using (var fs = new HyoutaUtils.Streams.DuplicatableFileStream(inputfilename)) {
					Dictionary<ushort, string> byIndex = new Dictionary<ushort, string>();
					if (sengame == 1) {
						var tbl = new SenLib.Sen1.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
						foreach (var e in tbl.Entries) {
							var vd = new SenLib.Sen1.FileFixes.VoiceData(e.Data, endian, encoding);
							if (!byIndex.ContainsKey(vd.Index)) {
								byIndex.Add(vd.Index, vd.Name);
							}
						}
					} else if (sengame == 2) {
						var tbl = new SenLib.Sen2.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
						foreach (var e in tbl.Entries) {
							// tbl header is different in CS2 but voice data payload is the same as CS1
							var vd = new SenLib.Sen1.FileFixes.VoiceData(e.Data, endian, encoding);
							if (!byIndex.ContainsKey(vd.Index)) {
								byIndex.Add(vd.Index, vd.Name);
							}
						}
					} else if (sengame == 3) {
						var tbl = new SenLib.Sen3.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
						foreach (var e in tbl.Entries) {
							var vd = new VoiceDataCS3(e.Data, endian, encoding);
							if (!byIndex.ContainsKey(vd.Index)) {
								byIndex.Add(vd.Index, vd.Name);
							}
						}
					} else if (sengame == 4) {
						var tbl = new SenLib.Sen4.Tbl(new HyoutaUtils.Streams.DuplicatableFileStream(voicetablefilename), endian, encoding);
						foreach (var e in tbl.Entries) {
							var vd = new VoiceDataCS4(e.Data, endian, encoding);
							if (!byIndex.ContainsKey(vd.Index)) {
								byIndex.Add(vd.Index, vd.Name);
							}
						}
					}

					var bytestream = fs.CopyToByteArrayStreamAndDispose();
					var sha1 = ChecksumUtils.CalculateSHA1ForEntireStream(bytestream);
					var funcs = ScriptParser.Parse(bytestream, false, byIndex, endian, sengame);
					using (var outfs = new FileStream(outputfilename, FileMode.Create)) {
						var sha1bytes = sha1.Value;
						outfs.WriteUTF8("new KnownFile(new SHA1(0x");
						for (int bidx = 0; bidx < 8; ++bidx) {
							outfs.WriteUTF8(sha1bytes[bidx].ToString("x2"));
						}
						outfs.WriteUTF8("ul, 0x");
						for (int bidx = 8; bidx < 16; ++bidx) {
							outfs.WriteUTF8(sha1bytes[bidx].ToString("x2"));
						}
						outfs.WriteUTF8("ul, 0x");
						for (int bidx = 16; bidx < 20; ++bidx) {
							outfs.WriteUTF8(sha1bytes[bidx].ToString("x2"));
						}
						outfs.WriteUTF8("u), \"");
						outfs.WriteUTF8(inputfilename.Replace('\\', '/'));
						outfs.WriteUTF8("\"),");
						outfs.WriteUTF8("\n");
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

			if (args.Length >= 1 && args[0] == "--check-voices") {
				string voice_tbl_path = args[1];
				string voice_file_path = args[2];
				sengame = int.Parse(args[3]);
				EndianUtils.Endianness endian = args[4] == "be" ? EndianUtils.Endianness.BigEndian : EndianUtils.Endianness.LittleEndian;
				TextUtils.GameTextEncoding encoding = args[5] == "sjis" ? TextUtils.GameTextEncoding.ShiftJIS : TextUtils.GameTextEncoding.UTF8;
				string output_path = args[6];
				t_voice_tbl.CheckVoiceTable(voice_tbl_path, voice_file_path, sengame, output_path, endian, encoding);
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

			if (File.Exists(System.IO.Path.Combine(path, "Sen1Launcher.exe"))) {
				sengame = 1;
			} else if (File.Exists(System.IO.Path.Combine(path, "Sen2Launcher.exe"))) {
				sengame = 2;
			} else if (File.Exists(System.IO.Path.Combine(path, "Sen3Launcher.exe"))) {
				sengame = 3;
			} else if (File.Exists(System.IO.Path.Combine(path, "Sen4Launcher.exe"))) {
				sengame = 4;
			} else {
				Console.WriteLine($"Failed to detect what game {path} is.");
				return -1;
			}

			FilenameFix.FixupIncorrectEncodingInFilenames(path, sengame, true, new CliProgressReporter());
			KnownFile[] knownFiles;
			switch (sengame) {
				case 1:
					knownFiles = Sen1KnownFiles.Files;
					break;
				case 2:
					knownFiles = Sen2KnownFiles.Files;
					break;
				case 3:
					knownFiles = Sen3KnownFiles.Files;
					break;
				case 4:
					knownFiles = Sen4KnownFiles.Files;
					break;
				default:
					return -1; // shouldn't get here
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
					showMouseCursor: true,
					disablePauseOnFocusLoss: true,
					fixArtsSupport: true,
					force0Kerning: true
				));
				mods.AddRange(Sen1Mods.GetAssetMods());
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else if (sengame == 2) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen2Mods.GetExecutableMods(
					Sen2Version.v142,
					removeTurboSkip: true,
					patchAudioThread: true,
					audioThreadDivisor: 1000,
					patchBgmQueueing: true,
					correctLanguageVoiceTables: true,
					disableMouseCapture: true,
					showMouseCursor: true,
					disablePauseOnFocusLoss: true,
					fixControllerMapping: true,
					fixArtsSupport: true,
					force0Kerning: true,
					fixBattleScopeCrash: true
				));
				mods.AddRange(Sen2Mods.GetAssetMods(Sen2Version.v142));
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else if (sengame == 3) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen3Mods.GetExecutableMods(
					fixInGameButtonMappingValidity: true,
					allowSwitchToNightmare: true,
					swapBrokenMasterQuartzValuesForDisplay: true,
					disableMouseCapture: true,
					showMouseCursor: true,
					disablePauseOnFocusLoss: true,
					fixControllerMapping: true
				));
				mods.AddRange(Sen3Mods.GetAssetMods(
					allowSwitchToNightmare: true
				));
				result = FileModExec.ExecuteMods(path, storage, mods, new CliProgressReporter());
			} else if (sengame == 4) {
				var mods = new List<FileMod>();
				mods.AddRange(Sen4Mods.GetExecutableMods(
					allowSwitchToNightmare: true,
					disableMouseCapture: true,
					showMouseCursor: true,
					disablePauseOnFocusLoss: true,
					separateSwapConfirmCancelOption: true,
					defaultSwapConfirmCancelOptionOn: true,
					fixSwappedButtonsWhenDynamicPromptsOff: true
				));
				mods.AddRange(Sen4Mods.GetAssetMods(
					allowSwitchToNightmare: true
				));
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
