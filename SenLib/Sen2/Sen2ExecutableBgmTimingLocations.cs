using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public class Sen2ExecutableBgmTimingLocations {
		public uint ThreadEntryPointAddress;
		public uint ThreadEntryPointLength;
		public uint ThreadFunctionBodyAddress;
		public uint ThreadFunctionBodyLength;

		public uint QueryPerformanceFrequency;
		public uint QueryPerformanceCounter;
		public uint LockMutex;
		public uint UnlockMutex;
		public uint InvokeSleepMilliseconds;
		public uint ProcessSoundQueue;
		public uint AllMul;
		public uint AllDvRm;

		public uint InnerLoopAddsd;
		public uint InnerLoopDivss;

		public uint BgmAlreadyPlayingJump;
		public uint MultiplierWhenLCrtlHeld;
		public uint MultiplierWhenLShiftHeld;


		public uint AddressOfDirtyFlag;
		public uint AddressOfOverwritableWriteSoundQueue4bytes_0x5;
		public uint AddressOfOverwritableWriteSoundQueue4bytes_0x6;
		public uint AddressWriteSoundQueue4bytes;
		public uint EndOfSoundQueueProcessing;
		public uint AddressOfIsPlayingCheckInjection;
	}
}
