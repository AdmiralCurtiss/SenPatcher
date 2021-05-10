using HyoutaUtils.Checksum;
using HyoutaUtils.Streams;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class FilenameFix {
		private class HashWithName {
			public SHA1 Hash;
			public string Name;
			public HashWithName(SHA1 hash, string name) { Hash = hash; Name = name; }
		}

		// CS1 on GoG, CS1 on Humble and CS2 on Humble ship with filenames in the wrong encoding
		// I suspect all of the affected files are actually unused, but might as well fix them up just in case...
		// If 'fix' is true we try to fix it if we determine that it's wrong, otherwise we just determine whether it's wrong or not.
		// return value should be interpreted as:
		// - if 'fix' is false -> whether there is anything that needs fixing (should call again with fix == true)
		// - if 'fix' is true  -> the correctness state of the files after this call; so if true we either found no issues or we fixed them all
		public static bool FixupIncorrectEncodingInFilenames(string gamepath, int sengame, bool fix, ProgressReporter progress) {
			try {
				if (sengame == 1) {
					bool s1 = FixEffectsSamples(gamepath, cs1: true, fix: fix, progress: progress);
					bool s2 = FixSoundEffectsCS1(gamepath, fix: fix, progress: progress);
					if (fix) {
						// report if everything worked
						return s1 && s2;
					} else {
						// report if at least one thing is in need of fixing
						return s1 || s2;
					}
				} else if (sengame == 2) {
					bool s1 = FixEffectsSamples(gamepath, cs1: false, fix: fix, progress: progress);
					bool s2 = FixNpc049CS2(gamepath, fix: fix, progress: progress);
					bool s3 = FixNpc610615CS2(gamepath, "data/chr/npc/npc610/BK", fix: fix, progress: progress);
					bool s4 = FixNpc610615CS2(gamepath, "data/chr/npc/npc615/BK", fix: fix, progress: progress);
					if (fix) {
						// report if everything worked
						return s1 && s2 && s3 && s4;
					} else {
						// report if at least one thing is in need of fixing
						return s1 || s2 || s3 || s4;
					}
				}
				// sengame is something unexpected
				return false;
			} catch (Exception) {
				// something broke; report that we can't fix this or that the fix didn't work
				return false;
			}
		}

		private static bool FixEffectsSamples(string gamepath, bool cs1, bool fix, ProgressReporter progress) {
			string subdir = "data/effects/samples";
			List<HashWithName> files = new List<HashWithName>();
			files.Add(new HashWithName(new SHA1(0x04225404e9334b1eul, 0x12d03a71841f5d16ul, 0x89ad10dbu), "\u7403(\u4e0a\u4e0b\u6d88\u3048).eff"));
			files.Add(new HashWithName(new SHA1(0x0dee9f4197976249ul, 0x24558dc3f8afeb0aul, 0xe877580cu), "dae\u304b\u3089\u30a8\u30d5\u30a7.eff"));
			files.Add(new HashWithName(new SHA1(0x35eaa8c4619455d4ul, 0x0b49d29db60de6aaul, 0x8dd2b7d9u), "\u91cd\u529b(\u885d\u7a81).eff"));
			files.Add(new HashWithName(new SHA1(0x462eeaf7ee00155dul, 0xc2851d0d2ac2dad9ul, 0x3d90241cu), "\u534a\u5186\u67f1.eff"));
			files.Add(new HashWithName(new SHA1(0x4ef7483f44589763ul, 0x748f3b5f48bb6809ul, 0xa330f582u), "\u30bf\u30fc\u30b2\u30c3\u30c8\u304b\u3089.eff"));
			files.Add(new HashWithName(new SHA1(0x638547eb41c7ac3cul, 0x9e3e577fbc3f6292ul, 0x43899e59u), "\u7403.eff"));
			files.Add(new HashWithName(new SHA1(0x986a81e94b981282ul, 0xa8629871e99a5999ul, 0x442ab071u), "\u5186\u67f1(\u4e0a\u4e0b\u6d88\u3048).eff"));
			files.Add(new HashWithName(new SHA1(0xb10361f42574696aul, 0xe0b7983bebded70eul, 0x04d52a52u), "\u91cd\u529b(\u8df3\u306d\u8fd4\u308a).eff"));
			files.Add(new HashWithName(new SHA1(0xc4fdbc028659a2c8ul, 0x048dbf0fc0a567b3ul, 0xcb52e888u), "\u81ea\u8ee2\u306b\u516c\u8ee2\u9069\u7528.eff"));
			files.Add(new HashWithName(new SHA1(0xc9b746303dbf8795ul, 0xdcfac5591d97c9faul, 0xb46b7678u), "\u30b9\u30c6\u30eb\u30b9\u98a8.eff"));
			files.Add(new HashWithName(new SHA1(0xcc1b4d4640ab06bcul, 0x6525ba2d1e88e86bul, 0xeac11ef0u), "\u30d1\u30bf\u30fc\u30f3\u30a2\u30cb\u30e1.eff"));
			files.Add(new HashWithName(new SHA1(0xed02bbc1a9bbe0d7ul, 0xbac82dd571d7ea86ul, 0xc66c65b1u), "\u8ecc\u8de1.eff"));
			files.Add(new HashWithName(new SHA1(0xf4097d4f84c73c16ul, 0x31272535cdbdc493ul, 0x64d99eceu), "\u7a32\u59bb.eff"));
			files.Add(new HashWithName(new SHA1(0xfcca406cdc4db09ful, 0x3a9f11a61db10b2ful, 0x51fe12e2u), "\u8ecc\u8de1(\u8981\uff71\uff80\uff6f\uff81).eff"));
			if (cs1) {
				files.Add(new HashWithName(new SHA1(0x5f6e1c9d3a56c87bul, 0xe91e3d55b0bc2849ul, 0xccd41baeu), "\u5341\u5b57.eff"));
			} else {
				files.Add(new HashWithName(new SHA1(0x0baccc654a461e6ful, 0x5bc8e9a32ac5a35cul, 0xac2dd0afu), "\u30ec\u30f3\u30ba\u30d5\u30ec\u30a2.eff"));
				files.Add(new HashWithName(new SHA1(0x2a5bdf1fb5f9adcful, 0xe0f189bc6105cbbful, 0x4919c5a6u), "\u592a\u967d.eff"));
				files.Add(new HashWithName(new SHA1(0x472c1dd01d8cd2ddul, 0xfd282a47f9770c7bul, 0xc0073892u), "\u5341\u5b57.eff"));
				files.Add(new HashWithName(new SHA1(0x73cb9aeede04b759ul, 0xebe3c611cf95482eul, 0x0756dbd5u), "\u30cb\u30a2\u30af\u30ea\u30c3\u30d7.eff"));
				files.Add(new HashWithName(new SHA1(0x77f257c73f555c19ul, 0x4a226a3a60344f50ul, 0xab366060u), "\u30d5\u30a9\u30b0\u30c1\u30a7\u30c3\u30af\u7528.eff"));
				files.Add(new HashWithName(new SHA1(0xbe93d2da9f1f10bdul, 0xb98c47be617b61f0ul, 0xbbd9ac75u), "\u30bd\u30d5\u30c8\u30d1\u30fc\u30c6\u30a3\u30af\u30eb.eff"));
			}

			string sampledir = Path.Combine(gamepath, subdir);
			if (!Directory.Exists(sampledir)) {
				// something is busted but it's not the filenames, we can't do anything about this
				return false;
			}

			foreach (FileSystemInfo f in new DirectoryInfo(sampledir).GetFileSystemInfos()) {
				string path;
				string name;
				bool deleteDir;
				if (f is DirectoryInfo) {
					// for the known encoding errors, this must always be a subdir with exactly one file in it
					var subdirinfos = ((DirectoryInfo)f).GetFileSystemInfos();
					if (subdirinfos.Length != 1 || !(subdirinfos[0] is FileInfo)) {
						continue;
					}
					path = subdirinfos[0].FullName;
					name = $"{f.Name}/{subdirinfos[0].Name}";
					deleteDir = true;
				} else if (f is FileInfo) {
					path = f.FullName;
					name = f.Name;
					deleteDir = false;
				} else {
					continue;
				}

				HashWithName target;
				using (DuplicatableFileStream fs = new DuplicatableFileStream(path)) {
					SHA1 hash = HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					target = files.FirstOrDefault(x => x.Hash == hash);
				}
				if (target != null && name != target.Name) {
					if (fix) {
						// this file should be elsewhere, try to fix this
						progress.Message(string.Format("Renaming {0} to {1}", path, target.Name));
						File.Move(path, Path.Combine(sampledir, target.Name));
						if (deleteDir) {
							progress.Message(string.Format("Deleting leftover folder at {0}", f.FullName));
							f.Delete();
						}
					} else {
						// this file should be elsewhere, report that we're in need of fixing
						progress.Message(string.Format("Found incorrect filename {0}, should be {1}", name, target.Name));
						return true;
					}
				}
			}

			if (fix) {
				// either we didn't encounter any issues or we fixed them
				return true;
			} else {
				// we didn't encounter any issues, so report that this folder does not need fixing
				return false;
			}
		}

		private static bool FixSoundEffectsCS1(string gamepath, bool fix, ProgressReporter progress) {
			string subdir = "data/se/wav";
			SHA1 targethash = new SHA1(0x684cc74b0837ff14ul, 0x08124f8b8a05cfd9ul, 0xc9a09195u);
			string targetName = "ed8\uff4d2123.wav";
			string asciiName = "ed8m2123.wav";
			string targetdir = Path.Combine(gamepath, subdir);
			if (!Directory.Exists(targetdir)) {
				// can't do anything here
				return false;
			}

			FileInfo asciiFileInfo = null;
			FileInfo otherFileInfo = null;
			foreach (FileInfo f in new DirectoryInfo(targetdir).GetFiles("ed8*2123.wav")) {
				SHA1? hash;
				using (DuplicatableFileStream fs = new DuplicatableFileStream(f.FullName)) {
					hash = HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(fs);
				}
				if (hash != null && hash.Value == targethash) {
					if (f.Name == targetName) {
						if (fix) {
							return true; // this is already fixed
						} else {
							return false; // does not need fixing
						}
					} else if (f.Name == asciiName) {
						asciiFileInfo = f;
					} else {
						otherFileInfo = f;
					}
				}
			}

			// if we reach here we know that the target file does not exist
			if (asciiFileInfo != null || otherFileInfo != null) {
				if (fix) {
					if (otherFileInfo != null) {
						// we have a broken encoding file, move it
						progress.Message(string.Format("Renaming {0} to {1}", otherFileInfo.FullName, targetName));
						File.Move(otherFileInfo.FullName, Path.Combine(targetdir, targetName));
					} else {
						// we have the ascii m file, this might be from a previous patch run, so just copy it
						progress.Message(string.Format("Copying {0} to {1}", asciiFileInfo.FullName, targetName));
						File.Copy(asciiFileInfo.FullName, Path.Combine(targetdir, targetName));
					}
				} else {
					// this is in need of fixing and we can fix it
					if (otherFileInfo != null) {
						progress.Message(string.Format("Found incorrect filename {0}, should be {1}", otherFileInfo.Name, targetName));
					} else {
						progress.Message(string.Format("Missing file {0}, but can be copied from {1}", targetName, asciiFileInfo.Name));
					}
					return true;
				}
			}

			// we can't fix this
			return false;
		}

		private static bool FixNpc049CS2(string gamepath, bool fix, ProgressReporter progress) {
			string subdir = "data/chr/npc";
			string targetdir = Path.Combine(gamepath, subdir);
			if (!Directory.Exists(targetdir)) {
				return false;
			}

			// there should be exactly one directory not starting with 'npc' in here, find it
			DirectoryInfo notnpc = null;
			foreach (DirectoryInfo d in new DirectoryInfo(targetdir).EnumerateDirectories()) {
				if (!d.Name.StartsWith("npc", StringComparison.InvariantCultureIgnoreCase)) {
					if (notnpc != null) {
						// this is the second dir, we have a problem
						return false;
					}
					notnpc = d;
				}
			}
			if (notnpc == null) {
				// couldn't find any dir, give up
				return false;
			}

			// check if we have the right dir
			string targetname = "\u51fa\u756a\u7121\u3057";
			if (targetname == notnpc.Name) {
				// this folder already has the correct name, nothing do to here
				if (fix) {
					return true;
				} else {
					// we didn't encounter any issues, so report that this folder does not need fixing
					return false;
				}
			} else {
				SHA1? hash;
				string infpath = Path.Combine(notnpc.FullName, "npc049/npc049.inf");
				if (File.Exists(infpath)) {
					using (DuplicatableFileStream fs = new DuplicatableFileStream(infpath)) {
						hash = HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(fs);
					}
					if (hash == new SHA1(0x6a2a9964df308b23ul, 0x307e436580efd036ul, 0x67c0ac08u)) {
						if (fix) {
							progress.Message(string.Format("Renaming {0} to {1}", notnpc.FullName, targetname));
							Directory.Move(notnpc.FullName, Path.Combine(gamepath, subdir, targetname));
							// we fixed it
							return true;
						} else {
							// this is in need of fixing
							progress.Message(string.Format("Found incorrect folder {0}, should be {1}", notnpc.FullName, targetname));
							return true;
						}
					} else {
						// not the right file, can't fix anything here
						return false;
					}
				} else {
					// not the right directory, we can't fix anything here
					return false;
				}
			}
		}

		private static bool FixNpc610615CS2(string gamepath, string subdir, bool fix, ProgressReporter progress) {
			string targetdir = Path.Combine(gamepath, subdir);
			if (!Directory.Exists(targetdir)) {
				// can't do anything here
				return false;
			}

			var dirinfos = new DirectoryInfo(targetdir).GetDirectories();
			if (dirinfos.Length != 1) {
				// there should only be one subdirectory here, something is weird
				return false;
			}
			var dirinfo = dirinfos[0];
			string targetname = "\u6ca1_\u30ac\u30c8\u30ea\u30f3\u30b0\u30ac\u30f3";
			if (targetname == dirinfo.Name) {
				// we're good
				if (fix) {
					return true;
				} else {
					// nothing to fix
					return false;
				}
			}

			string infpath = Path.Combine(dirinfo.FullName, "equ610.inf");
			if (File.Exists(infpath)) {
				SHA1? hash;
				using (DuplicatableFileStream fs = new DuplicatableFileStream(infpath)) {
					hash = HyoutaUtils.ChecksumUtils.CalculateSHA1ForEntireStream(fs);
				}
				if (hash == new SHA1(0xe773a19f67feba62ul, 0xbb16ef2112eab24dul, 0xc3b7bd83u)) {
					if (fix) {
						progress.Message(string.Format("Renaming {0} to {1}", dirinfo.FullName, targetname));
						Directory.Move(dirinfo.FullName, Path.Combine(gamepath, subdir, targetname));
						// fixed it
						return true;
					} else {
						// in need of fixing
						progress.Message(string.Format("Found incorrect folder {0}, should be {1}", dirinfo.FullName, targetname));
						return true;
					}
				} else {
					// file is not what we expected, can't fix this
					return false;
				}
			} else {
				// file doesn't exist, clearly we have the wrong folder, can't do anything
				return false;
			}
		}
	}
}
