using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchMusicQueueing(Stream binary, Sen2ExecutablePatchInterface patchInfo, Sen2ExecutablePatchState state) {
			var mapper = new Sen2Mapper();
			var a = patchInfo.GetBgmTimingPatchLocations();

			state.InitCodeSpaceIfNeeded(binary, patchInfo);

			using (BranchHelper4Byte lock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte unlock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte set_dirty_and_write_sound_queue_4bytes = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte clear_dirty_if_queue_empty = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte queue_not_empty = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte queue_empty_jump_back = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte write_sound_queue_4bytes = new BranchHelper4Byte(binary, mapper)) {
				EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
				EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;
				Stream _ = binary;

				// end of the uninitialized .data section, right before .rsrc, should be unused
				// TODO: where is the actual information about the location/length of that section stored...?
				uint address_of_dirty_flag = 0x1179ffe;
				uint address_of_overwritable_write_sound_queue_4bytes_0x5 = 0x0041fdd7;
				uint address_of_overwritable_write_sound_queue_4bytes_0x6 = 0x004219b9;
				uint address_write_sound_queue_4bytes = 0x00422220;
				uint end_of_sound_queue_processing = 0x0041f1a8;

				write_sound_queue_4bytes.SetTarget(address_write_sound_queue_4bytes);
				lock_mutex.SetTarget(a.LockMutex);
				unlock_mutex.SetTarget(a.UnlockMutex);

				// set dirty flag when a BGM switch or stop is enqueued into the sound queue
				{
					_.Position = (long)mapper.MapRamToRom(state.RegionD.Address);
					set_dirty_and_write_sound_queue_4bytes.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0xb8);                            // mov eax,(address of dirty flag)
					_.WriteUInt32(address_of_dirty_flag, le);
					_.WriteUInt24(0xc60001, be);                   // mov byte ptr[eax],1
					write_sound_queue_4bytes.WriteJump5Byte(0xe9); // jmp write_sound_queue_4bytes
					state.RegionD.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(address_of_overwritable_write_sound_queue_4bytes_0x5);
					set_dirty_and_write_sound_queue_4bytes.WriteJump5Byte(0xe8);
				}
				{
					_.Position = (long)mapper.MapRamToRom(address_of_overwritable_write_sound_queue_4bytes_0x6);
					set_dirty_and_write_sound_queue_4bytes.WriteJump5Byte(0xe8);
				}

				// clear dirty flag after sound queue is processed if queue is still empty
				// this is... very much a heuristic, but should be *good enough* hopefully
				{
					_.Position = (long)mapper.MapRamToRom(end_of_sound_queue_processing);
					ulong tmp = _.PeekUInt48(le); // remember instruction here so we can reinsert it later
					clear_dirty_if_queue_empty.WriteJump5Byte(0xe9);
					_.WriteUInt8(0x90); // nop rest of instruction for safety, though shouldn't matter
					queue_empty_jump_back.SetTarget(mapper.MapRomToRam((ulong)_.Position));


					_.Position = (long)mapper.MapRamToRom(state.Region50a.Address);
					clear_dirty_if_queue_empty.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt48(0x8bb56cffffff, be);          // mov  esi,dword ptr [ebp-94h]
					_.WriteUInt24(0x8d4e44, be);                // lea  ecx,[esi+44h]
					lock_mutex.WriteJump5Byte(0xe8);            // call lock_mutex
					_.WriteUInt32(0x837e6c00, be);              // cmp  dword ptr [esi+6Ch],0
					queue_not_empty.WriteJump(0x75);            // jne  queue_not_empty
					_.WriteUInt8(0xb9);                         // mov  ecx,(address of dirty flag)
					_.WriteUInt32(address_of_dirty_flag, le);
					_.WriteUInt24(0xc60100, be);                // mov  byte ptr[ecx],0
					queue_not_empty.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8d4e44, be);                // lea  ecx,[esi+44h]
					unlock_mutex.WriteJump5Byte(0xe8);          // call unlock_mutex
					_.WriteUInt48(tmp, le);                     // reinsert instruction we overwrote
					queue_empty_jump_back.WriteJump5Byte(0xe9); // jmp  queue_empty_jump_back
					state.Region50a.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}

				// TODO: patch the code at ~0x57c80f to check whether the bgm is fading as well as the dirty flag


				// 0x8eda84 -> FSound vftable
				// 0x8ed0b4 -> FSoundBase vftable
				// 0x8eda10 -> FSoundChannelController vftable
				// 0x8ed268 -> FSoundBank vftable

				// FSound data structure:
				// +0x00 vftableptr
				// +0x04 ?
				// +0x08 length of 1st FSoundChannelController* array
				// +0x0C pointer to 1st FSoundChannelController* array
				// +0x10 length of 2nd FSoundChannelController* array
				// +0x14 pointer to 2nd FSoundChannelController* array
				// +0x44 mutex handle for locking the sound queue (?)
				// +0x5C base address of sound queue (ringbuffer?)
				// +0x60 size of sound queue
				// +0x64 write offset in sound queue (?)
				// +0x68 read offset in sound queue (?)
				// +0x6C bytes used in sound queue


				// always jump the disallow-enqueue-while-same-track-playing branch
				// if we need a safer test, *(int*)(*(((int*)edi)+5)) in function at 0x41F846 (which is the currently-playing-bgm check, called at 0x57c803)
				// seems to give us the pointer to the data structure containing the current volume/fade info of the bgm
				// which seems to be:
				//    +0x00 ptr: vftable
				//    +0x04 ?
				//    +0x08 ?
				//    +0x0c ?
				//    +0x10 ?
				//    +0x14 ?
				//    +0x18 ?
				//    +0x1C float: ?
				//    +0x20 float: current volume
				//    +0x24 float: fade start factor
				//    +0x28 float: fade end factor
				//    +0x2C float: target fade time in seconds
				//    +0x30 float: current fade time in seconds
				//    +0x34 float?: ?
				//    +0x38 byte: fade out is active?
				//    +0x4D byte: channel is active?
				// (compare the fade adjustment function at 0x421da0, which is pretty clear)
				// so we could inject a test for this at 0x57c80f to allow fades if and only if bgm is currently fading out
				// by checking (current fade time < target fade time) && fade end factor == 0.0f
				// but this doesn't actually seem to be necessary, as far as I can tell?
				// still, figured I'd note this here in case it ends up being useful
				_.Position = (long)mapper.MapRamToRom(a.BgmAlreadyPlayingJump);
				_.WriteUInt8(0xeb);
			}
		}
	}
}
