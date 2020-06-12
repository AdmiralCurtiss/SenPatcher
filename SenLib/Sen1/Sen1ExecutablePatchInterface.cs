using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public interface Sen1ExecutablePatchInterface {
		// address of button ID used to activate turbo mode when held
		long GetAddressButtonTurboMode();

		// address of button ID held to auto-skip animation in battle (usually the same as turbo mode)
		long GetAddressButtonBattleAnimationAutoSkip();

		// address of button ID held to auto-skip battle results (usually the same as turbo mode)
		long GetAddressButtonBattleResultsAutoSkip();

		// address of conditional jump statement for auto-skipping animations when turbo button is held
		long GetAddressJumpBattleAnimationAutoSkip();

		// address of conditional jump statement for auto-skipping results when turbo button is held
		long GetAddressJumpBattleResultsAutoSkip();

		// address of conditional jump statement for not checking the R2 notebook shortcut when turbo is enabled
		long GetAddressJumpR2NotebookOpen();

		// address of conditional jump statement for not displaying the R2 option in the settings when turbo is enabled
		long GetAddressJumpR2NotebookSettings();

		// address of extra character in the middle of texture ID string of Thor MQ, for the HQ texture pack
		long GetRomAddressThorMasterQuartzTextureIdTypo();

		// get fixes to be applied to files other than the executable
		List<FileFix> GetFileFixes();
	}
}
