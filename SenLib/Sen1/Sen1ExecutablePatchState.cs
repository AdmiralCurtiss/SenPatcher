using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen1 {
	public partial class Sen1ExecutablePatchState {
		public bool IsJp { get; private set; }

		public HyoutaPluginBase.IRomMapper Mapper { get; private set; }

		// address of button ID held to auto-skip animation in battle (usually the same as turbo mode)
		public long AddressButtonBattleAnimationAutoSkip { get; private set; }

		// address of button ID held to auto-skip battle results (usually the same as turbo mode)
		public long AddressButtonBattleResultsAutoSkip { get; private set; }

		// address of button ID used to activate turbo mode when held
		public long AddressButtonTurboMode { get; private set; }

		// address of conditional jump statement for auto-skipping animations when turbo button is held
		public long AddressJumpBattleAnimationAutoSkip { get; private set; }

		// address of conditional jump statement for auto-skipping results when turbo button is held
		public long AddressJumpBattleResultsAutoSkip { get; private set; }

		// address of conditional jump statement for not checking the R2 notebook shortcut when turbo is enabled
		public long AddressJumpR2NotebookOpen { get; private set; }

		// address of conditional jump statement for not displaying the R2 option in the settings when turbo is enabled
		public long AddressJumpR2NotebookSettings { get; private set; }

		// address of a conditional that jumps over setting the font kerning value to 0
		public long AddressForce0Kerning { get; private set; }

		// address of extra character in the middle of texture ID string of Thor MQ, for the HQ texture pack
		public long RomAddressThorMasterQuartzTextureIdTypo { get; private set; }

		// push address of version string displayed on the title screen
		public long PushAddressVersionString { get; private set; }

		public Sen1ExecutablePatchState(bool jp, HyoutaPluginBase.IRomMapper mapper) {
			IsJp = jp;
			Mapper = mapper;

			if (jp) {
				AddressButtonBattleAnimationAutoSkip = 0x4d6409;
				AddressButtonBattleResultsAutoSkip = 0x4f0a27;
				AddressButtonTurboMode = 0x48179a;
				AddressJumpBattleAnimationAutoSkip = 0x4d6411;
				AddressJumpBattleResultsAutoSkip = 0x4f0a2f;
				AddressJumpR2NotebookOpen = 0x5b6fbf;
				AddressJumpR2NotebookSettings = 0x6de1a0;
				RomAddressThorMasterQuartzTextureIdTypo = 0x738f6a;
				PushAddressVersionString = 0x68e93d;
			} else {
				AddressButtonBattleAnimationAutoSkip = 0x4d7c59;
				AddressButtonBattleResultsAutoSkip = 0x4f2247;
				AddressButtonTurboMode = 0x48300a;
				AddressJumpBattleAnimationAutoSkip = 0x4d7c61;
				AddressJumpBattleResultsAutoSkip = 0x4f224f;
				AddressJumpR2NotebookOpen = 0x5b812f;
				AddressJumpR2NotebookSettings = 0x6dfaf0;
				AddressForce0Kerning = 0x5a0631;
				RomAddressThorMasterQuartzTextureIdTypo = 0x73adba;
				PushAddressVersionString = 0x69042d;
			}

			CtorCodeSpaceScriptCompilerDummy(jp);
		}

		public void InitCodeSpaceIfNeeded(System.IO.Stream binary) {
			InitCodeSpaceScriptCompilerDummyIfNeeded(binary);
		}
	}
}
