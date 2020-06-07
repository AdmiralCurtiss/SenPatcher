using HyoutaUtils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SenLib.Sen2 {
	public static partial class Sen2ExecutablePatches {
		public static void PatchMusicFadeTiming(Stream binary, Sen2ExecutablePatchInterface patchInfo, Sen2ExecutablePatchState state, uint divisor) {
			var mapper = new Sen2Mapper();
			var a = patchInfo.GetBgmTimingPatchLocations();

			// divisor of 1000 seems to be console-accurate, but making fades a little faster actually feels nicer with the fast PC loading times

			state.InitCodeSpaceIfNeeded(binary, patchInfo);
			RegionHelper region50b = state.Region50b;
			RegionHelper region51 = state.Region51;
			RegionHelper regionEntryPoint = null;
			RegionHelper region80 = null;

			using (BranchHelper4Byte alldvrm = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte allmul = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte do_compare = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte invoke_sleep_milliseconds = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte lock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte invoke_query_performance_counter = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte invoke_query_performance_frequency = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte process_sound_queue = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte unlock_mutex = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte do_compare_end = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte fail = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte success = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte early_exit = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte exit_inner_loop = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte go_to_sleep_maybe = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte go_to_next_iteration = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte inner_loop = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte outer_loop_init = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte outer_loop = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte post_every_33_iterations = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte remainder_increment = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte thread_mainloop_continue = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper4Byte time_pass_loop_4byte = new BranchHelper4Byte(binary, mapper))
			using (BranchHelper1Byte time_pass_loop_1byte = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper1Byte exit_remainder_increment = new BranchHelper1Byte(binary, mapper))
			using (BranchHelper4Byte thread_mainloop = new BranchHelper4Byte(binary, mapper)) {
				{
					// this is the entry point of the function we're replacing, clear it out
					binary.Position = (long)mapper.MapRamToRom((ulong)a.ThreadEntryPointAddress);
					for (uint i = 0; i < a.ThreadEntryPointLength; ++i) {
						binary.WriteUInt8(0xcc);
					}
					regionEntryPoint = new RegionHelper(a.ThreadEntryPointAddress, a.ThreadEntryPointLength, "Timing Thread Entry Point");
				}
				{
					// this is the body of the function we're replacing, clear it out
					// don't ask me why this is split up like this
					binary.Position = (long)mapper.MapRamToRom((ulong)a.ThreadFunctionBodyAddress);
					for (uint i = 0; i < a.ThreadFunctionBodyLength; ++i) {
						binary.WriteUInt8(0xcc);
					}
					region80 = new RegionHelper(a.ThreadFunctionBodyAddress, a.ThreadFunctionBodyLength, "Timing Thread Body");
				}

				EndianUtils.Endianness be = EndianUtils.Endianness.BigEndian;
				EndianUtils.Endianness le = EndianUtils.Endianness.LittleEndian;

				// and assemble!
				Stream _ = binary;
				{
					_.Position = (long)mapper.MapRamToRom(regionEntryPoint.Address);
					_.WriteUInt8(0x55);                         // push       ebp
					_.WriteUInt16(0x8bec, be);                  // mov        ebp,esp
					_.WriteUInt24(0x83ec7c, be);                // sub        esp,7Ch
					_.WriteUInt24(0x8b4d08, be);                // mov        ecx,dword ptr [ebp+8]
					_.WriteUInt8(0x57);                         // push       edi
					_.WriteUInt8(0x56);                         // push       esi
					thread_mainloop.WriteJump5Byte(0xe9);       // jmp        thread_mainloop
					regionEntryPoint.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region41.Address);
					remainder_increment.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8b45bc, be);                // mov        eax,dword ptr [ebp-44h]  ; load counter into eax
					_.WriteUInt16(0x03c2, be);                  // add        eax,edx                  ; counter += ticks_per_loop_remainder
					_.WriteUInt24(0x8b55c8, be);                // mov        edx,dword ptr [ebp-38h]  ; edx = original_divisor
					_.WriteUInt16(0x3bc2, be);                  // cmp        eax,edx                  ; if counter >= original_divisor
					exit_remainder_increment.WriteJump(0x72);   // jb         exit_remainder_increment
					_.WriteUInt32(0x8345f001, be);              // add        dword ptr [ebp-10h],1    ; ++ticks_last
					_.WriteUInt32(0x8355f400, be);              // adc        dword ptr [ebp-0Ch],0
					_.WriteUInt16(0x2bc2, be);                  // sub        eax,edx                  ; counter -= original_divisor
					exit_remainder_increment.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8945bc, be);                // mov        dword ptr [ebp-44h],eax  ; write counter back to stack
					time_pass_loop_4byte.WriteJump5Byte(0xe9);  // jmp        time_pass_loop
					state.Region41.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region41.Address);
					thread_mainloop.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x53);                                      // push       ebx
					_.WriteUInt16(0x8bf9, be);                               // mov        edi,ecx
					_.WriteUInt16(0x33f6, be);                               // xor        esi,esi
					_.WriteUInt56(0xf74710ffffff7f, be);                     // test       dword ptr [edi+10h],7FFFFFFFh
					early_exit.WriteJump6Byte(0x0f84);                       // je         early_exit
					_.WriteUInt24(0x8d45d8, be);                             // lea        eax,[ebp-28h]
					invoke_query_performance_frequency.WriteJump5Byte(0xe8); // call       invoke_query_performance_frequency
					thread_mainloop_continue.WriteJump5Byte(0xe9);           // jmp        thread_mainloop_continue
					state.Region41.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(region51.Address);
					thread_mainloop_continue.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0xb8);                         // mov        eax,(divisor)
					_.WriteUInt32(divisor, le);
					_.WriteUInt24(0x8945c8, be);                // mov        dword ptr [ebp-38h],eax
					_.WriteUInt16(0x6a00, be);                  // push       0
					_.WriteUInt8(0x50);                         // push       eax
					_.WriteUInt24(0x8b45dc, be);                // mov        eax,dword ptr [ebp-24h]
					_.WriteUInt24(0x8b55d8, be);                // mov        edx,dword ptr [ebp-28h]
					_.WriteUInt8(0x50);                         // push       eax
					_.WriteUInt8(0x52);                         // push       edx
					alldvrm.WriteJump5Byte(0xe8);               // call       _alldvrm
					_.WriteUInt24(0x8945e0, be);                // mov        dword ptr [ebp-20h],eax
					_.WriteUInt24(0x8955e4, be);                // mov        dword ptr [ebp-1Ch],edx
					_.WriteUInt24(0x894dc0, be);                // mov        dword ptr [ebp-40h],ecx

					_.WriteUInt24(0x8b45dc, be);                // mov        eax,dword ptr [ebp-24h]
					_.WriteUInt24(0x8b55d8, be);                // mov        edx,dword ptr [ebp-28h]
					_.WriteUInt16(0x6a00, be);                  // push       0
					_.WriteUInt16(0x6a05, be);                  // push       5
					_.WriteUInt8(0x50);                         // push       eax
					_.WriteUInt8(0x52);                         // push       edx
					allmul.WriteJump5Byte(0xe8);                // call       _allmul
					_.WriteUInt24(0x8945d0, be);                // mov        dword ptr [ebp-30h],eax
					_.WriteUInt24(0x8955d4, be);                // mov        dword ptr [ebp-2Ch],edx

					_.WriteUInt24(0x8d45f0, be);                           // lea        eax,[ebp-10h]
					invoke_query_performance_counter.WriteJump5Byte(0xe8); // call       invoke_query_performance_counter
					_.WriteUInt32(0x807f5400, be);                         // cmp        byte ptr [edi+54h],0
					early_exit.WriteJump6Byte(0x0f85);                     // jne        early_exit
					outer_loop_init.WriteJump5Byte(0xe9);                  // jmp        outer_loop_init
					region51.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(region80.Address);
					outer_loop_init.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt56(0xc745bc00000000, be);                   // mov        dword ptr [ebp-44h],0
					outer_loop.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt16(0x33db, be);                             // xor        ebx,ebx
					_.WriteUInt24(0x8d45e8, be);                           // lea        eax,[ebp-18h]
					invoke_query_performance_counter.WriteJump5Byte(0xe8); // call       invoke_query_performance_counter
					_.WriteUInt24(0x8d45e8, be);                           // lea        eax,[ebp-18h] ; ticks_now
					_.WriteUInt24(0x8d55d0, be);                           // lea        edx,[ebp-30h] ; ticks_for_reset
					_.WriteUInt24(0x8d4df0, be);                           // lea        ecx,[ebp-10h] ; ticks_last
					do_compare.WriteJump5Byte(0xe8);                       // call       do_compare
					_.WriteUInt16(0x85c0, be);                             // test       eax,eax
					time_pass_loop_1byte.WriteJump(0x74);                  // je         time_pass_loop
					_.WriteUInt24(0x8b45e8, be);                           // mov        eax,dword ptr [ebp-18h]
					_.WriteUInt24(0x8945f0, be);                           // mov        dword ptr [ebp-10h],eax
					_.WriteUInt24(0x8b45ec, be);                           // mov        eax,dword ptr [ebp-14h]
					_.WriteUInt24(0x8945f4, be);                           // mov        dword ptr [ebp-0Ch],eax
					time_pass_loop_1byte.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					time_pass_loop_4byte.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8d45e8, be);                           // lea        eax,[ebp-18h] ; ticks_now
					_.WriteUInt24(0x8d55e0, be);                           // lea        edx,[ebp-20h] ; ticks_per_loop
					_.WriteUInt24(0x8d4df0, be);                           // lea        ecx,[ebp-10h] ; ticks_last
					do_compare.WriteJump5Byte(0xe8);                       // call       do_compare
					_.WriteUInt16(0x85c0, be);                             // test       eax,eax
					go_to_sleep_maybe.WriteJump(0x74);                     // je         go_to_sleep_maybe
					inner_loop.WriteJump5Byte(0xe9);                       // jmp        inner_loop
					exit_inner_loop.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8b45e0, be);                           // mov        eax,dword ptr [ebp-20h] ; ticks_per_loop
					_.WriteUInt24(0x8b4de4, be);                           // mov        ecx,dword ptr [ebp-1Ch]
					_.WriteUInt24(0x0145f0, be);                           // add        dword ptr [ebp-10h],eax ; ticks_last
					_.WriteUInt24(0x114df4, be);                           // adc        dword ptr [ebp-0Ch],ecx
					_.WriteUInt24(0x8b55c0, be);                           // mov        edx,dword ptr [ebp-40h] ; ticks_per_loop_remainder
					_.WriteUInt16(0x85d2, be);                             // test       edx,edx
					time_pass_loop_1byte.WriteJump(0x74);                  // je         time_pass_loop ; no remainder, just go back to loop
					remainder_increment.WriteJump5Byte(0xe9);              // jmp        remainder_increment
					go_to_sleep_maybe.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt16(0x85db, be);                             // test       ebx,ebx
					go_to_next_iteration.WriteJump(0x75);                  // jne        go_to_next_iteration
					_.WriteUInt16(0x6a00, be);                             // push       0
					invoke_sleep_milliseconds.WriteJump5Byte(0xe8);        // call       invoke_sleep_milliseconds
					_.WriteUInt24(0x83c404, be);                           // add        esp,4
					go_to_next_iteration.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt32(0x807f5400, be);                         // cmp        byte ptr [edi+54h],0
					outer_loop.WriteJump(0x74);                            // je         outer_loop
					early_exit.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x5b);                                    // pop        ebx
					_.WriteUInt8(0x5e);                                    // pop        esi
					_.WriteUInt16(0x33c0, be);                             // xor        eax,eax
					_.WriteUInt8(0x5f);                                    // pop        edi
					_.WriteUInt16(0x8be5, be);                             // mov        esp,ebp
					_.WriteUInt8(0x5d);                                    // pop        ebp
					_.WriteUInt8(0xc3);                                    // ret
					region80.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(region50b.Address);
					inner_loop.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt40(0xbb01000000, be);                       // mov        ebx,1 ; remember that we've executed an inner_loop
					_.WriteUInt24(0x8d4f38, be);                           // lea        ecx,[edi+38h]
					lock_mutex.WriteJump5Byte(0xe8);                       // call       lock_mutex
					_.WriteUInt16(0x8bcf, be);                             // mov        ecx,edi
					process_sound_queue.WriteJump5Byte(0xe8);              // call       unknown_func
					_.WriteUInt24(0x83fe21, be);                           // cmp        esi,21h
					post_every_33_iterations.WriteJump(0x72);              // jb         post_every_33_iterations
					_.WriteUInt32(0x660f6ec6, be);                         // movd       xmm0,esi
					_.WriteUInt24(0x0f5bc0, be);                           // cvtdq2ps   xmm0,xmm0
					_.WriteUInt16(0x8b17, be);                             // mov        edx,dword ptr [edi]
					_.WriteUInt8(0x51);                                    // push       ecx
					_.WriteUInt16(0x8bcf, be);                             // mov        ecx,edi
					_.WriteUInt32(0xf30f5e05, be);                         // divss      xmm0,dword ptr ds:[8ED254h]
					_.WriteUInt32(a.InnerLoopDivss, le);
					_.WriteUInt40(0xf30f594758, be);                       // mulss      xmm0,dword ptr [edi+58h]
					_.WriteUInt40(0xf30f110424, be);                       // movss      dword ptr [esp],xmm0
					_.WriteUInt24(0xff5268, be);                           // call       dword ptr [edx+68h]
					_.WriteUInt24(0x83ee21, be);                           // sub        esi,21h
					post_every_33_iterations.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt24(0x8d4f38, be);                           // lea        ecx,[edi+38h]
					unlock_mutex.WriteJump5Byte(0xe8);                     // call       unlock_mutex
					_.WriteUInt8(0x46);                                    // inc        esi
					exit_inner_loop.WriteJump5Byte(0xe9);                  // jmp        exit_inner_loop
					region50b.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.RegionD.Address);
					invoke_query_performance_frequency.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x50);                // push eax
					_.WriteUInt16(0xff15, be);         // call dword ptr[QueryPerformanceFrequency]
					_.WriteUInt32(a.QueryPerformanceFrequency);
					_.WriteUInt8(0xc3);                // ret
					state.RegionD.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region32.Address);
					invoke_query_performance_counter.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x50);                // push eax
					_.WriteUInt16(0xff15, be);         // call dword ptr[QueryPerformanceCounter]
					_.WriteUInt32(a.QueryPerformanceCounter);
					_.WriteUInt8(0xc3);                // ret
					state.Region32.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}
				{
					_.Position = (long)mapper.MapRamToRom(state.Region32.Address);
					do_compare.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x53);                         // push        ebx
					_.WriteUInt16(0x8bd8, be);                  // mov         ebx,eax
					_.WriteUInt24(0x8b4104, be);                // mov         eax,dword ptr [ecx+4]
					_.WriteUInt8(0x56);                         // push        esi
					_.WriteUInt16(0x8b31, be);                  // mov         esi,dword ptr [ecx]
					_.WriteUInt16(0x0332, be);                  // add         esi,dword ptr [edx]
					_.WriteUInt16(0x8bcb, be);                  // mov         ecx,ebx
					_.WriteUInt24(0x134204, be);                // adc         eax,dword ptr [edx+4]
					_.WriteUInt24(0x3b4104, be);                // cmp         eax,dword ptr [ecx+4]
					success.WriteJump(0x7f);                    // jg          success
					fail.WriteJump(0x7c);                       // jl          fail
					_.WriteUInt16(0x3b31, be);                  // cmp         esi,dword ptr [ecx]
					success.WriteJump(0x73);                    // jae         success
					fail.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt40(0xb801000000, be);            // mov         eax,1
					do_compare_end.WriteJump(0xeb);             // jmp         do_compare_end
					success.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt16(0x33c0, be);                  // xor         eax,eax
					do_compare_end.SetTarget(mapper.MapRomToRam((ulong)_.Position));
					_.WriteUInt8(0x5e);                         // pop         esi
					_.WriteUInt8(0x5b);                         // pop         ebx
					_.WriteUInt8(0xc3);                         // ret
					state.Region32.TakeToAddress((long)mapper.MapRomToRam((ulong)_.Position));
				}

				lock_mutex.SetTarget(a.LockMutex);
				unlock_mutex.SetTarget(a.UnlockMutex);
				invoke_sleep_milliseconds.SetTarget(a.InvokeSleepMilliseconds);
				process_sound_queue.SetTarget(a.ProcessSoundQueue);
				allmul.SetTarget(a.AllMul);
				alldvrm.SetTarget(a.AllDvRm);

				// patch the bizarre and apparently intentional behavior that messes with the music timing when left shift or left ctrl are held
				_.Position = (long)mapper.MapRamToRom(a.MultiplierWhenLCrtlHeld);
				_.WriteUInt32(0x3f800000, le);
				_.Position = (long)mapper.MapRamToRom(a.MultiplierWhenLShiftHeld);
				_.WriteUInt32(0x3f800000, le);
			}
		}
	}
}
