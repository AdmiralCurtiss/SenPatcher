using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public interface ProgressReporter {
		void Message(string msg, int current = -1, int total = -1);
		void Error(string msg);
		void Finish(bool success);
	}

	public class DummyProgressReporter : ProgressReporter {
		public void Message(string msg, int current = -1, int total = -1) { }
		public void Error(string msg) { }
		public void Finish(bool success) { }
	}

	public class CliProgressReporter : ProgressReporter {
		public void Message(string msg, int current = -1, int total = -1) {
			if (current != -1 && total != -1) {
				Console.WriteLine("Progress: {0}/{1}", current, total);
			}
			Console.WriteLine(msg);
		}

		public void Error(string msg) {
			Console.WriteLine("ERROR: {0}", msg);
		}

		public void Finish(bool success) {
			Console.WriteLine("Task finished with {0}.", success ? "success" : "failure");
		}
	}
}
