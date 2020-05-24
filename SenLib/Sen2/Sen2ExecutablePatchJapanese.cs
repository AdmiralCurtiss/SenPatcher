using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutablePatchJapanese : Sen2ExecutablePatchInterface {
		public long GetAddressJumpBattleAnimationAutoSkip() {
			return 0x479b1b;
		}

		public long GetAddressJumpBattleResultsAutoSkip() {
			return 0x4929ba;
		}

		public long GetAddressJumpBattleSomethingAutoSkip() {
			return 0x4845ae;
		}

		public long GetAddressJumpBattleStartAutoSkip() {
			return 0x4836eb;
		}
	}
}
