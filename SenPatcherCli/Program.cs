using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenPatcherCli {
	public class Program {
		public static void Main(string[] args) {
			bool sen1 = false;
			if (sen1) {
				using (var outstream = new FileStream(@"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel\ed8.exe", FileMode.Create)) {
					using (var instream = new FileStream(@"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel\ed8.exe.senpatcher.bkp", FileMode.Open, FileAccess.Read)) {
						StreamUtils.CopyStream(instream, outstream);
					}

					var info = new SenLib.Sen1.Sen1ExecutablePatchEnglish();
					SenLib.Sen1.Sen1ExecutablePatches.PatchButtonBattleAnimationAutoSkip(outstream, info, 0xA);
					SenLib.Sen1.Sen1ExecutablePatches.PatchButtonBattleResultsAutoSkip(outstream, info, 0xA);
					SenLib.Sen1.Sen1ExecutablePatches.PatchButtonTurboMode(outstream, info, 0xA);
					SenLib.Sen1.Sen1ExecutablePatches.PatchJumpBattleAnimationAutoSkip(outstream, info, true);
					SenLib.Sen1.Sen1ExecutablePatches.PatchJumpBattleResultsAutoSkip(outstream, info, true);
					SenLib.Sen1.Sen1ExecutablePatches.PatchJumpR2NotebookOpen(outstream, info, true);
					SenLib.Sen1.Sen1ExecutablePatches.PatchJumpR2NotebookSettings(outstream, info, true);
				}
			}

			bool sen2 = true;
			if (sen2) {
				using (var outstream = new FileStream(@"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel II\bin\Win32\ed8_2_PC_US.exe", FileMode.Create)) {
					using (var instream = new FileStream(@"c:\Program Files (x86)\Steam\steamapps\common\Trails of Cold Steel II\bin\Win32\ed8_2_PC_US.exe.senpatcher.bkp", FileMode.Open, FileAccess.Read)) {
						StreamUtils.CopyStream(instream, outstream);
					}

					var info = new SenLib.Sen2.Sen2ExecutablePatchEnglish();
					SenLib.Sen2.Sen2ExecutablePatches.PatchJumpBattleAnimationAutoSkip(outstream, info, true);
					SenLib.Sen2.Sen2ExecutablePatches.PatchJumpBattleStartAutoSkip(outstream, info, true);
					SenLib.Sen2.Sen2ExecutablePatches.PatchJumpBattleSomethingAutoSkip(outstream, info, true);
					SenLib.Sen2.Sen2ExecutablePatches.PatchJumpBattleResultsAutoSkip(outstream, info, true);

					var state = new SenLib.Sen2.Sen2ExecutablePatchState();
					SenLib.Sen2.Sen2ExecutablePatches.PatchMusicFadeTiming(outstream, info, state, 1000);
					SenLib.Sen2.Sen2ExecutablePatches.PatchMusicQueueingOnSoundThreadSide(outstream, info, state);
					state.PrintStatistics();
				}
			}
		}
	}
}
