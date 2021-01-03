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
	}
}
