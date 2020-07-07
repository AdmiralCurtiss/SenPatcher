using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public class Sen1ExecutablePatchJapanese : Sen1ExecutablePatchInterface {
		public long GetAddressButtonBattleAnimationAutoSkip() {
			return 0x4d6409;
		}

		public long GetAddressButtonBattleResultsAutoSkip() {
			return 0x4f0a27;
		}

		public long GetAddressButtonTurboMode() {
			return 0x48179a;
		}

		public long GetAddressJumpBattleAnimationAutoSkip() {
			return 0x4d6411;
		}

		public long GetAddressJumpBattleResultsAutoSkip() {
			return 0x4f0a2f;
		}

		public long GetAddressJumpR2NotebookOpen() {
			return 0x5b6fbf;
		}

		public long GetAddressJumpR2NotebookSettings() {
			return 0x6de1a0;
		}

		public long GetRomAddressThorMasterQuartzTextureIdTypo() {
			return 0x738f6a;
		}

		public List<FileFix> GetFileFixes() {
			var f = new List<FileFix>();
			f.Add(new FileFixes.se_wav_ed8m2123_wav());
			return f;
		}
	}
}
