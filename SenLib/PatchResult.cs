using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class PatchResult {
		public int TotalFiles;
		public int SuccessfulFiles;

		public bool AllSuccessful => TotalFiles == SuccessfulFiles;

		public int FailedFiles => TotalFiles - SuccessfulFiles;

		public PatchResult(int total, int successful) {
			TotalFiles = total;
			SuccessfulFiles = successful;
		}
	}
}
