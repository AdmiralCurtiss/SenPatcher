using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public class FileModResult {
		// path the file should be written to
		public string TargetPath;

		// data that should be written
		// if set to null, this indicates the file should be deleted instead
		public Stream ResultData;

		public FileModResult(string path, Stream data) {
			TargetPath = path;
			ResultData = data;
		}
	}
}
