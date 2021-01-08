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
			if (File.Exists(path)) {
				try {
					using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read)) {
						if (fs.Length == stream.Length) {
							if (fs.CopyToByteArray().SequenceEqual(stream.CopyToByteArray())) {
								return true; // file is already the same, don't need to write
							}
						}
					}
				} catch (Exception) { }
			}

			return TryWriteFile(stream, path);
		}

		public static bool TryWriteFile(Stream stream, string path) {
			long pos = stream.Position;
			try {
				using (var fs = new FileStream(path, FileMode.Create, FileAccess.Write)) {
					stream.Position = 0;
					StreamUtils.CopyStream(stream, fs);
				}
				stream.Position = pos;
				return true;
			} catch (Exception) {
				stream.Position = pos;
				return false;
			}
		}

		public static bool TryDeleteFile(string path) {
			if (!File.Exists(path)) {
				return true;
			}

			try {
				File.Delete(path);
				return true;
			} catch (Exception) {
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

		public static void JumpOverCode(RegionHelper region, HyoutaPluginBase.IRomMapper mapper, Stream bin) {
			using (BranchHelper1Byte helper = new BranchHelper1Byte(bin, mapper)) {
				bin.Position = mapper.MapRamToRom(region.Address);
				helper.WriteJump(0xeb);
				region.TakeToAddress(mapper.MapRomToRam(bin.Position), "jump over");
				helper.SetTarget(region.Address + region.Remaining);
			}
		}
	}
}
