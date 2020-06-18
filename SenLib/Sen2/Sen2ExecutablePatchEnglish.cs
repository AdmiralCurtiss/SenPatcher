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

		public Sen2ExecutableCodeSpaceLocations GetCodeSpaceLocations() {
			var c = new Sen2ExecutableCodeSpaceLocations();
			c.Region50aAddress = 0x420c4b; // command 0x9
			c.Region50aTarget = 0x4219cb; // command 0x6
			c.Region50aLength = 0x50;
			c.Region50bAddress = 0x4211fb; // command 0xB
			c.Region50bTarget = 0x4219cb; // command 0x6
			c.Region50bLength = 0x50;
			c.Region51Address = 0x42143a; // command 0x4
			c.Region51Target = 0x42161b; // command 0xA
			c.Region51Length = 0x51;
			c.Region60Address = 0x420d5b; // command 0x7
			c.Region60Target = 0x42160b; // command 0xA
			c.Region60Length = 0x60;

			c.Region41Address = 0x421b2a; // command 0x2
			c.Region41Target = 0x4219db; // command 0x6
			c.Region41Length = 0x41;
			c.Region32Address = 0x41fcc9; // command 0x0
			c.Region32Target = 0x420f51; // command 0x8
			c.Region32Length = 0x32;

			c.RegionDAddress = 0x41fc03;
			c.RegionDLength = 0xd;

			return c;
		}

		public Sen2ExecutableBgmTimingLocations GetBgmTimingPatchLocations() {
			var a = new Sen2ExecutableBgmTimingLocations();
			a.ThreadEntryPointAddress = 0x41dc30;
			a.ThreadEntryPointLength = 0x10;
			a.ThreadFunctionBodyAddress = 0x421f10;
			a.ThreadFunctionBodyLength = 0x80;
			a.QueryPerformanceFrequency = 0x8ea11c;
			a.QueryPerformanceCounter = 0x8ea120;
			a.LockMutex = 0x71e550;
			a.UnlockMutex = 0x71e580;
			a.InvokeSleepMilliseconds = 0x71de50;
			a.ProcessSoundQueue = 0x41e9f0;
			a.AllMul = 0x816b40;
			a.AllDvRm = 0x88e340;
			a.InnerLoopAddsd = 0x8ec240; // this one probably actually not necessary but let's not mess with this too much...
			a.InnerLoopDivss = 0x8ed254;
			a.BgmAlreadyPlayingJump = 0x57c80d;
			a.MultiplierWhenLCrtlHeld = 0x581f66;
			a.MultiplierWhenLShiftHeld = 0x581f7c;

			a.AddressOfDirtyFlag = 0x1179ffc;
			a.AddressOfOverwritableWriteSoundQueue4bytes_0x5 = 0x41fdd7;
			a.AddressOfOverwritableWriteSoundQueue4bytes_0x6 = 0x4219b9;
			a.AddressWriteSoundQueue4bytes = 0x422220;
			a.EndOfSoundQueueProcessing = 0x41f1a8;
			a.AddressOfIsPlayingCheckInjection = 0x57c7fb;

			a.AddressOfSkipEnqueueOnSoundThreadSideInjection = 0x41ef37;
			return a;
		}

		public List<FileFix> GetFileFixes() {
			var f = new List<FileFix>();
			f.Add(new FileFixes.text_dat_us_t_magic_tbl());
			return f;
		}
	}
}
