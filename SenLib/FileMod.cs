using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public interface FileMod {
		// should return null if apply fails
		// otherwise returns the data and the relative-to-basepath path of the patched files
		IEnumerable<FileModResult> TryApply(FileStorage storage);

		// like TryApply() but returns the operations needed to revert to the unmodified game state
		IEnumerable<FileModResult> TryRevert(FileStorage storage);

		// human-readable description of this patch
		string GetDescription();
	}
}
