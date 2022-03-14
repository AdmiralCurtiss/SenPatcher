using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public enum FileModResultType {
		File,
		CreateFolder,
		DeleteFolder,
	}

	public class FileModResult {
		public FileModResultType Type;

		// path of the file or folder
		public string TargetPath;

		// File only: data that should be written
		// if set to null, this indicates the file should be deleted instead
		public Stream ResultData;

		public FileModResult(string path, Stream data) {
			Type = FileModResultType.File;
			TargetPath = path;
			ResultData = data;
		}

		public FileModResult(string path, FileModResultType folderType) {
			if (folderType == FileModResultType.File) {
				throw new Exception();
			}
			Type = folderType;
			TargetPath = path;
			ResultData = null;
		}
	}
}
