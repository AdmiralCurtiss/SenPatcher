using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class SenUtils {
		public static bool TryWriteFileIfDifferent(Stream stream, string path) {
			SenLib.Logging.Log(string.Format("Writing file to {0} if different...", path));

			if (File.Exists(path)) {
				try {
					using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
						if (fs.Length == stream.Length) {
							if (fs.CopyToByteArray().SequenceEqual(stream.CopyToByteArray())) {
								SenLib.Logging.Log(string.Format("File {0} already identical with intended write.", path));
								return true; // file is already the same, don't need to write
							}
						}
					}
				} catch (Exception ex) {
					SenLib.Logging.Log(string.Format("Error while checking file {0} for difference: {1}", path, ex.ToString()));
				}
			}

			return TryWriteFile(stream, path);
		}

		public static bool TryWriteFile(Stream stream, string path) {
			SenLib.Logging.Log(string.Format("Writing file to {0}...", path));

			long pos = stream.Position;
			try {
				using (var fs = new FileStream(path, FileMode.Create, FileAccess.Write)) {
					stream.Position = 0;
					StreamUtils.CopyStream(stream, fs);
				}
				stream.Position = pos;
				SenLib.Logging.Log(string.Format("Successfully wrote file {0}", path));
				return true;
			} catch (Exception ex) {
				stream.Position = pos;
				SenLib.Logging.Log(string.Format("Error while writing file {0}: {1}", path, ex.ToString()));
				return false;
			}
		}

		public static bool TryDeleteFile(string path) {
			SenLib.Logging.Log(string.Format("Deleting {0} if exists...", path));
			if (!File.Exists(path)) {
				SenLib.Logging.Log(string.Format("{0} does not exist.", path));
				return true;
			}

			try {
				File.Delete(path);
				SenLib.Logging.Log(string.Format("Successfully deleted {0}", path));
				return true;
			} catch (Exception ex) {
				SenLib.Logging.Log(string.Format("Error while deleting file {0}: {1}", path, ex.ToString()));
				return false;
			}
		}

		public static bool TryCreateDirectory(string path) {
			try {
				if (Directory.Exists(path)) {
					return true;
				}
				Directory.CreateDirectory(path);
				return true;
			} catch (Exception ex) {
				SenLib.Logging.Log(string.Format("Error while creating directory {0}: {1}", path, ex.ToString()));
				return false;
			}
		}

		public static bool TryDeleteEmptyDirectory(string path) {
			try {
				if (!Directory.Exists(path)) {
					return true;
				}
				Directory.Delete(path);
				return true;
			} catch (Exception ex) {
				SenLib.Logging.Log(string.Format("Error while deleting directory {0}: {1}", path, ex.ToString()));
				return false;
			}
		}

		public static string ExtractUserFriendlyStringFromModDescriptions(List<FileMod> mods) {
			StringBuilder sb = new StringBuilder();
			foreach (FileMod mod in mods) {
				sb.AppendLine(mod.GetDescription());
			}
			return sb.ToString();
		}

		public static RegionHelper InitRegion(string id, long ramstart, long ramend, HyoutaPluginBase.IRomMapper mapper, Stream bin, byte b) {
			long romstart = mapper.MapRamToRom(ramstart);
			long romend = mapper.MapRamToRom(ramend);
			bin.Position = romstart;
			for (long i = romstart; i < romend; ++i) {
				bin.WriteUInt8(b);
			}
			return new RegionHelper((uint)ramstart, (uint)(ramend - ramstart), id);
		}

		public static RegionHelper64 InitRegion64(string id, long ramstart, long ramend, HyoutaPluginBase.IRomMapper mapper, Stream bin, byte b) {
			long romstart = mapper.MapRamToRom(ramstart);
			long romend = mapper.MapRamToRom(ramend);
			bin.Position = romstart;
			for (long i = romstart; i < romend; ++i) {
				bin.WriteUInt8(b);
			}
			return new RegionHelper64(ramstart, (uint)(ramend - ramstart), id);
		}

		public static void JumpOverCode(RegionHelper region, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			using (BranchHelper1Byte helper = new BranchHelper1Byte(bin, mapper)) {
				bin.Position = mapper.MapRamToRom(region.Address);
				helper.WriteJump(0xeb);
				region.TakeToAddress(mapper.MapRomToRam(bin.Position), "jump over");
				helper.SetTarget(region.Address + region.Remaining);
			}
		}

		public static void JumpOverCode(RegionHelper64 region, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			using (BranchHelper1Byte helper = new BranchHelper1Byte(bin, mapper)) {
				bin.Position = mapper.MapRamToRom(region.Address);
				helper.WriteJump(0xeb);
				region.TakeToAddress(mapper.MapRomToRam(bin.Position), "jump over");
				helper.SetTarget((ulong)(region.Address + region.Remaining));
			}
		}

		public static void WriteRelativeAddress32(long absoluteTarget, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			long absoluteHere = mapper.MapRomToRam(bin.Position + 4);
			bin.WriteUInt32((uint)(int)(absoluteTarget - absoluteHere), EndianUtils.Endianness.LittleEndian);
		}

		public static (byte jmpbyte, long address) ReadJump5Byte_x64(Stream bin, HyoutaPluginBase.IRomMapper mapper) {
			byte jmpbyte = bin.ReadUInt8();
			long relativeAddress = bin.ReadInt32(EndianUtils.Endianness.LittleEndian);
			return (jmpbyte, relativeAddress + mapper.MapRomToRam(bin.Position));
		}
	}
}
