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
	}
}
