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

		public Sen2ExecutableCodeSpaceLocations GetCodeSpaceLocations() {
			var c = new Sen2ExecutableCodeSpaceLocations();
			c.Region50aAddress = 0x420bbb;
			c.Region50aTarget = 0x42195b;
			c.Region50aLength = 0x50;
			c.Region50bAddress = 0x42117b;
			c.Region50bTarget = 0x42195b;
			c.Region50bLength = 0x50;
			c.Region51Address = 0x4213ba;
			c.Region51Target = 0x42159b;
			c.Region51Length = 0x51;
			c.Region60Address = 0x420ccb;
			c.Region60Target = 0x42158b;
			c.Region60Length = 0x60;

			c.Region41Address = 0x421aba; // command 0x2
			c.Region41Target = 0x42196b; // command 0x6
			c.Region41Length = 0x41;
			c.Region32Address = 0x41fca9; // command 0x0
			c.Region32Target = 0x420ed1; // command 0x8
			c.Region32Length = 0x32;

			c.RegionDAddress = 0x41fbe3;
			c.RegionDLength = 0xd;

			return c;
		}

		public Sen2ExecutableBgmTimingLocations GetBgmTimingPatchLocations() {
			var a = new Sen2ExecutableBgmTimingLocations();
			a.ThreadEntryPointAddress = 0x41dc10;
			a.ThreadEntryPointLength = 0x10;
			a.ThreadFunctionBodyAddress = 0x421ea0;
			a.ThreadFunctionBodyLength = 0x80;
			a.QueryPerformanceFrequency = 0x8e9120;
			a.QueryPerformanceCounter = 0x8e9124;
			a.LockMutex = 0x71d4f0;
			a.UnlockMutex = 0x71d520;
			a.InvokeSleepMilliseconds = 0x71cdf0;
			a.ProcessSoundQueue = 0x41e9d0;
			a.AllMul = 0x815af0;
			a.AllDvRm = 0x88d2c0;
			a.InnerLoopAddsd = 0x8eb170;
			a.InnerLoopDivss = 0x8ebfd8;
			a.BgmAlreadyPlayingJump = 0x57c49d;
			a.MultiplierWhenLCrtlHeld = 0x581ba6;
			a.MultiplierWhenLShiftHeld = 0x581bbc;

			a.AddressOfDirtyFlag = 0x1174ffc;
			a.AddressOfOverwritableWriteSoundQueue4bytes_0x5 = 0x41fdb7;
			a.AddressOfOverwritableWriteSoundQueue4bytes_0x6 = 0x421949;
			a.AddressWriteSoundQueue4bytes = 0x4221a0;
			a.EndOfSoundQueueProcessing = 0x41f188;
			a.AddressOfIsPlayingCheckInjection = 0x57c48b;

			a.AddressOfSkipEnqueueOnSoundThreadSideInjection = 0x41ef17;
			return a;
		}

		public List<FileFix> GetFileFixes() {
			return new List<FileFix>();
		}
	}
}
