using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutableBgmTimingLocations {
		public uint ThreadEntryPointAddress { get; private set; }
		public uint ThreadEntryPointLength { get; private set; }
		public uint ThreadFunctionBodyAddress { get; private set; }
		public uint ThreadFunctionBodyLength { get; private set; }

		public uint QueryPerformanceFrequency { get; private set; }
		public uint QueryPerformanceCounter { get; private set; }
		public uint LockMutex { get; private set; }
		public uint UnlockMutex { get; private set; }
		public uint InvokeSleepMilliseconds { get; private set; }
		public uint ProcessSoundQueue { get; private set; }
		public uint AllMul { get; private set; }
		public uint AllDvRm { get; private set; }

		public uint InnerLoopAddsd { get; private set; }
		public uint InnerLoopDivss { get; private set; }

		public uint BgmAlreadyPlayingJump { get; private set; }
		public uint MultiplierWhenLCrtlHeld { get; private set; }
		public uint MultiplierWhenLShiftHeld { get; private set; }


		public uint AddressOfDirtyFlag { get; private set; }
		public uint AddressOfOverwritableWriteSoundQueue4bytes_0x5 { get; private set; }
		public uint AddressOfOverwritableWriteSoundQueue4bytes_0x6 { get; private set; }
		public uint AddressWriteSoundQueue4bytes { get; private set; }
		public uint EndOfSoundQueueProcessing { get; private set; }
		public uint AddressOfIsPlayingCheckInjection { get; private set; }

		public uint AddressOfSkipEnqueueOnSoundThreadSideInjection { get; private set; }

		public Sen2ExecutableBgmTimingLocations(bool jp) {
			if (jp) {
				ThreadEntryPointAddress = 0x41dc10;
				ThreadEntryPointLength = 0x10;
				ThreadFunctionBodyAddress = 0x421ea0;
				ThreadFunctionBodyLength = 0x80;
				QueryPerformanceFrequency = 0x8e9120;
				QueryPerformanceCounter = 0x8e9124;
				LockMutex = 0x71d4f0;
				UnlockMutex = 0x71d520;
				InvokeSleepMilliseconds = 0x71cdf0;
				ProcessSoundQueue = 0x41e9d0;
				AllMul = 0x815af0;
				AllDvRm = 0x88d2c0;
				InnerLoopAddsd = 0x8eb170;
				InnerLoopDivss = 0x8ebfd8;
				BgmAlreadyPlayingJump = 0x57c49d;
				MultiplierWhenLCrtlHeld = 0x581ba6;
				MultiplierWhenLShiftHeld = 0x581bbc;

				AddressOfDirtyFlag = 0x1174ffc;
				AddressOfOverwritableWriteSoundQueue4bytes_0x5 = 0x41fdb7;
				AddressOfOverwritableWriteSoundQueue4bytes_0x6 = 0x421949;
				AddressWriteSoundQueue4bytes = 0x4221a0;
				EndOfSoundQueueProcessing = 0x41f188;
				AddressOfIsPlayingCheckInjection = 0x57c48b;

				AddressOfSkipEnqueueOnSoundThreadSideInjection = 0x41ef17;
			} else {
				ThreadEntryPointAddress = 0x41dc30;
				ThreadEntryPointLength = 0x10;
				ThreadFunctionBodyAddress = 0x421f10;
				ThreadFunctionBodyLength = 0x80;
				QueryPerformanceFrequency = 0x8ea11c;
				QueryPerformanceCounter = 0x8ea120;
				LockMutex = 0x71e550;
				UnlockMutex = 0x71e580;
				InvokeSleepMilliseconds = 0x71de50;
				ProcessSoundQueue = 0x41e9f0;
				AllMul = 0x816b40;
				AllDvRm = 0x88e340;
				InnerLoopAddsd = 0x8ec240; // this one probably actually not necessary but let's not mess with this too much...
				InnerLoopDivss = 0x8ed254;
				BgmAlreadyPlayingJump = 0x57c80d;
				MultiplierWhenLCrtlHeld = 0x581f66;
				MultiplierWhenLShiftHeld = 0x581f7c;

				AddressOfDirtyFlag = 0x1179ffc;
				AddressOfOverwritableWriteSoundQueue4bytes_0x5 = 0x41fdd7;
				AddressOfOverwritableWriteSoundQueue4bytes_0x6 = 0x4219b9;
				AddressWriteSoundQueue4bytes = 0x422220;
				EndOfSoundQueueProcessing = 0x41f1a8;
				AddressOfIsPlayingCheckInjection = 0x57c7fb;

				AddressOfSkipEnqueueOnSoundThreadSideInjection = 0x41ef37;
			}
		}
	}
}
