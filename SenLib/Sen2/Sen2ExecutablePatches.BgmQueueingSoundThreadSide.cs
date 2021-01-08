using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchMusicQueueingOnSoundThreadSide(Stream binary, Sen2ExecutablePatchState state) {
			var mapper = state.Mapper;
			var a = state.BgmTimingPatchLocations;

			state.InitCodeSpaceIfNeeded(binary);

			using (BranchHelper4Byte lock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte unlock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte inject_entry_point = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte enqueue_check = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte exit_continue = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte exit_skip = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte back_to_function = new BranchHelper4Byte(binary, mapper)) {
				EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
				EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;
				Stream _ = binary;

				lock_mutex.SetTarget(a.LockMutex);
				unlock_mutex.SetTarget(a.UnlockMutex);

				// ignore the check if the track is already playing on main thread side
				{
					_.Position = (long)mapper.MapRamToRom(a.BgmAlreadyPlayingJump);
					_.WriteUInt8(0xeb);

					// for testing, write command even if the tracked track is the same
					//_.Position = (long)mapper.MapRamToRom(0x57c836);
					//_.WriteUInt8(0x90);
					//_.WriteUInt8(0x90);
				}

				// on audio thread side, inject after the bgm play command is extracted from the ringbuffer
				// but before it is actually used to switch to the BGM
				{
					_.Position = (long)mapper.MapRamToRom(a.AddressOfSkipEnqueueOnSoundThreadSideInjection);
					inject_entry_point.WriteJump5Byte(0xe9);
					back_to_function.SetTarget(mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region50a.Address);
					inject_entry_point.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt56(0xf74610ffffff7f, be);       // test dword ptr[esi+10h],7FFFFFFFh
					enqueue_check.WriteJump6Byte(0x0f85);      // jne  enqueue_check
					_.WriteUInt16(0x8bce, be);                 // mov  ecx,esi
					_.WriteUInt24(0xff5034, be);               // call dword ptr[eax+34h]
					back_to_function.WriteJump5Byte(0xe9);     // jmp  back_to_function
					state.Region50a.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position), "BGM Queueing Audio Thread Side: Inject Entry");
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region60.Address);

					enqueue_check.SetTarget(mapper.MapRomToRam((ulong)_.Position));

					_.WriteUInt8(0x57);                     // push edi
					_.WriteUInt24(0x8d4e40, be);            // lea  ecx,[esi+40h]
					lock_mutex.WriteJump5Byte(0xe8);        // call lock_mutex
					_.WriteUInt24(0x8b4614, be);            // mov  eax,dword ptr[esi+14h]
					_.WriteUInt32(0x8b4c241c, be);          // mov  ecx,dword ptr[esp+1Ch]
					_.WriteUInt24(0x8b3c88, be);            // mov  edi,dword ptr[eax+ecx*4h]

					_.WriteUInt16(0x8b07, be);              // mov  eax,dword ptr[edi]
					_.WriteUInt16(0x8bcf, be);              // mov  ecx,edi
					_.WriteUInt24(0x8b4010, be);            // mov  eax,dword ptr[eax+10h]
					_.WriteUInt16(0xffd0, be);              // call eax
					_.WriteUInt16(0x84c0, be);              // test al,al
					exit_continue.WriteJump(0x74);          // je   exit_continue

					_.WriteUInt24(0x8b4708, be);            // mov  eax,dword ptr[edi+8h]
					_.WriteUInt16(0x85c0, be);              // test eax,eax
					exit_continue.WriteJump(0x74);          // je   exit_continue
					_.WriteUInt24(0x8b404c, be);            // mov  eax,dword ptr[eax+4Ch] 
					_.WriteUInt48(0x8b8dccfeffff, be);      // mov  ecx,dword ptr[ebp-134h]
					_.WriteUInt16(0x3bc1, be);              // cmp  eax,ecx
					exit_continue.WriteJump(0x75);          // jne  exit_continue

					_.WriteUInt24(0x8a4738, be);            // mov  al,byte ptr[edi+38h]
					_.WriteUInt16(0x84c0, be);              // test al,al
					exit_continue.WriteJump(0x75);          // jne  exit_continue

					_.WriteUInt8(0x5f);                     // pop  edi
					_.WriteUInt24(0x83c41c, be);            // add  esp,1Ch
					exit_skip.WriteJump(0xeb);              // jmp  exit_skip

					exit_continue.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x5f);                     // pop  edi
					_.WriteUInt16(0x8b06, be);              // mov  eax,dword ptr[esi]
					_.WriteUInt16(0x8bce, be);              // mov  ecx,esi
					_.WriteUInt24(0xff5034, be);            // call dword ptr[eax+34h]

					exit_skip.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8d4e40, be);            // lea  ecx,[esi+40h]
					unlock_mutex.WriteJump5Byte(0xe8);      // call unlock_mutex
					back_to_function.WriteJump5Byte(0xe9);  // jmp  back_to_function

					state.Region60.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position), "BGM Queueing Audio Thread Side: Inject Main");
				}
			}
		}
	}
}
