using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutablePatchEnglish : Sen2ExecutablePatchInterface {
		public long GetAddressJumpBattleAnimationAutoSkip() {
			return 0x479c8b;
		}

		public long GetAddressJumpBattleResultsAutoSkip() {
			return 0x492bea;
		}

		public long GetAddressJumpBattleSomethingAutoSkip() {
			return 0x48474e;
		}

		public long GetAddressJumpBattleStartAutoSkip() {
			return 0x48388b;
		}
	}
}
