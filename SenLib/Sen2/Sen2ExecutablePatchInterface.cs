using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public interface Sen2ExecutablePatchInterface {
		// address of conditional jump statement for auto-skipping animations when turbo button is held
		long GetAddressJumpBattleAnimationAutoSkip();

		// address of conditional jump statement for auto-skipping the start-of-battle camera sweep when turbo button is held
		long GetAddressJumpBattleStartAutoSkip();

		// address of conditional jump statement for auto-skipping... something when turbo button is held
		// not 100% sure what this actually is, but I've seen it between a tutorial box and the combat menu
		// popping up in a divine knight battle, at the very least
		long GetAddressJumpBattleSomethingAutoSkip();

		// address of conditional jump statement for auto-skipping results when turbo button is held
		long GetAddressJumpBattleResultsAutoSkip();

		// addresses of regions that can be repurposed for extra code space
		Sen2ExecutableCodeSpaceLocations GetCodeSpaceLocations();

		// a whole bunch of addresses for the bgm timing patch
		Sen2ExecutableBgmTimingLocations GetBgmTimingPatchLocations();
	}
}
