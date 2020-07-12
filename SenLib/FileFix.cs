using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib {
	public interface FileFix {
		bool TryApply(string basepath, string backuppath);

		bool TryRevert(string basepath, string backuppath);

		string GetDescription();
	}
}
