using System;
using System.Collections.Generic;

namespace SenLib {
	public class FileModInvert : FileMod {
		private FileMod Mod;

		public FileModInvert(FileMod mod) {
			Mod = mod;
		}

		public string GetDescription() {
			return Mod.GetDescription();
		}

		public IEnumerable<FileModResult> TryApply(FileStorage storage) {
			return Mod.TryRevert(storage);
		}

		public IEnumerable<FileModResult> TryRevert(FileStorage storage) {
			return Mod.TryApply(storage);
		}
	}
}
