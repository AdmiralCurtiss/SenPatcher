using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public interface ProgressReporter {
		void Message(string msg);
	}

	public class DummyProgressReporter : ProgressReporter {
		public void Message(string msg) { }
	}

	public class CliProgressReporter : ProgressReporter {
		public void Message(string msg) {
			Console.WriteLine(msg);
		}
	}
}
