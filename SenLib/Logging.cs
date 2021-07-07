using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public static class Logging {
		public static bool LogToConsole = false;
		public static bool LogToFile = false;

		private static object Lock = new object();
		private static FileStream File = null;

		public static void Log(
			string s,
			[CallerMemberName] string memberName = "",
			[CallerFilePath] string sourceFilePath = "",
			[CallerLineNumber] int sourceLineNumber = 0
		) {
			LogForwarded(s, memberName, sourceFilePath, sourceLineNumber);
		}

		public static void LogForwarded(string s, string memberName, string sourceFilePath, int sourceLineNumber) {
			LogInternal(s, LogToConsole, LogToFile, memberName, sourceFilePath, sourceLineNumber);
		}

		public static void LogForwardedForceConsole(string s, string memberName, string sourceFilePath, int sourceLineNumber) {
			LogInternal(s, true, LogToFile, memberName, sourceFilePath, sourceLineNumber);
		}

		private static void LogInternal(string s, bool console, bool file, string memberName, string sourceFilePath, int sourceLineNumber) {
			lock (Lock) {
				if (console) {
					Console.WriteLine(s);
				}
				if (file) {
					if (File == null) {
						File = new FileStream("senpatcher.log", FileMode.Append, FileAccess.Write);
						File.WriteUTF8(string.Format("\n\n===== Starting logging session at {0} =====\n", DateTime.UtcNow.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture)));
					}
					File.WriteUTF8(string.Format("[{1}:{2} {0}()] {3}\n", memberName, sourceFilePath, sourceLineNumber, s));
				}
			}
		}

		public static void Flush() {
			lock (Lock) {
				if (File != null) {
					File.Flush();
				}
			}
		}
	}
}
