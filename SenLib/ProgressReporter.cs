using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public interface ProgressReporter {
		void Message(string msg, int current = -1, int total = -1, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0);
		void Error(string msg, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0);
		void Finish(bool success, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0);
	}

	public class DummyProgressReporter : ProgressReporter {
		public void Message(string msg, int current = -1, int total = -1, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			if (current != -1 && total != -1) {
				Logging.LogForwarded(string.Format("Progress: {0}/{1}", current, total), memberName, sourceFilePath, sourceLineNumber);
			}
			Logging.LogForwarded(msg, memberName, sourceFilePath, sourceLineNumber);
		}

		public void Error(string msg, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			Logging.LogForwarded(string.Format("ERROR: {0}", msg), memberName, sourceFilePath, sourceLineNumber);
		}

		public void Finish(bool success, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			Logging.LogForwarded(string.Format("Task finished with {0}.", success ? "success" : "failure"), memberName, sourceFilePath, sourceLineNumber);
		}
	}

	public class CliProgressReporter : ProgressReporter {
		public void Message(string msg, int current = -1, int total = -1, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			if (current != -1 && total != -1) {
				Logging.LogForwardedForceConsole(string.Format("Progress: {0}/{1}", current, total), memberName, sourceFilePath, sourceLineNumber);
			}
			Logging.LogForwardedForceConsole(msg, memberName, sourceFilePath, sourceLineNumber);
		}

		public void Error(string msg, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			Logging.LogForwardedForceConsole(string.Format("ERROR: {0}", msg), memberName, sourceFilePath, sourceLineNumber);
		}

		public void Finish(bool success, [CallerMemberName] string memberName = "", [CallerFilePath] string sourceFilePath = "", [CallerLineNumber] int sourceLineNumber = 0) {
			Logging.LogForwardedForceConsole(string.Format("Task finished with {0}.", success ? "success" : "failure"), memberName, sourceFilePath, sourceLineNumber);
		}
	}
}
