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
		public static string CalcSha1(Stream s) {
			using (SHA1 sha1 = SHA1.Create()) {
				long p = s.Position;
				s.Position = 0;
				byte[] hashbytes = sha1.ComputeHash(s);
				s.Position = p;
				StringBuilder sb = new StringBuilder(28);
				foreach (byte b in hashbytes) {
					sb.Append(b.ToString("x2"));
				}
				return sb.ToString();
			}
		}

		public static void CreateBackupIfRequired(string backuppath, Stream stream) {
			// if the backup already exists no need to write it
			if (File.Exists(backuppath)) {
				try {
					using (var fs = new FileStream(backuppath, FileMode.Open, FileAccess.Read, FileShare.Read)) {
						stream.Position = 0;
						if (fs.Length == stream.Length) {
							if (fs.CopyToByteArray().SequenceEqual(stream.CopyToByteArray())) {
								return;
							}
						}
					}
				} catch (Exception) { }
			}

			// backup doesn't exist, or mismatches, or some other weird issue, so just create it
			Directory.CreateDirectory(Path.GetDirectoryName(backuppath));
			using (var fs = new FileStream(backuppath, FileMode.Create, FileAccess.Write)) {
				stream.Position = 0;
				StreamUtils.CopyStream(stream, fs);
			}
		}

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
	}
}
