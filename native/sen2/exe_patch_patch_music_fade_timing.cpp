#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchMusicFadeTiming(HyoutaUtils::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd,
                          uint32_t divisor) {
    using namespace SenPatcher::x86;

    // divisor of 1000 seems to be console-accurate

    char* const ThreadEntryPointAddress =
        GetCodeAddressJpEn(version, textRegion, 0x41dc10, 0x41dc30);
    const size_t ThreadEntryPointLength = 0x10;
    char* const ThreadFunctionBodyAddress =
        GetCodeAddressJpEn(version, textRegion, 0x421ea0, 0x421f10);
    const size_t ThreadFunctionBodyLength = 0x80;
    char* const QueryPerformanceFrequency =
        GetCodeAddressJpEn(version, textRegion, 0x8e9120, 0x8ea11c);
    char* const QueryPerformanceCounter =
        GetCodeAddressJpEn(version, textRegion, 0x8e9124, 0x8ea120);
    char* const LockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d4f0, 0x71e550);
    char* const UnlockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d520, 0x71e580);
    char* const InvokeSleepMilliseconds =
        GetCodeAddressJpEn(version, textRegion, 0x71cdf0, 0x71de50);
    char* const ProcessSoundQueue = GetCodeAddressJpEn(version, textRegion, 0x41e9d0, 0x41e9f0);
    char* const AllMul = GetCodeAddressJpEn(version, textRegion, 0x815af0, 0x816b40);
    char* const AllDvRm = GetCodeAddressJpEn(version, textRegion, 0x88d2c0, 0x88e340);
    char* const InnerLoopDivss = GetCodeAddressJpEn(version, textRegion, 0x8ebfd8, 0x8ed254);

    BranchHelper4Byte alldvrm;
    BranchHelper4Byte allmul;
    BranchHelper4Byte do_compare;
    BranchHelper4Byte invoke_sleep_milliseconds;
    BranchHelper4Byte lock_mutex;
    BranchHelper4Byte invoke_query_performance_counter;
    BranchHelper4Byte invoke_query_performance_frequency;
    BranchHelper4Byte process_sound_queue;
    BranchHelper4Byte unlock_mutex;
    BranchHelper1Byte do_compare_end;
    BranchHelper1Byte fail;
    BranchHelper1Byte success;
    BranchHelper4Byte early_exit;
    BranchHelper4Byte exit_inner_loop;
    BranchHelper1Byte go_to_sleep_maybe;
    BranchHelper1Byte go_to_next_iteration;
    BranchHelper4Byte inner_loop;
    BranchHelper4Byte outer_loop_init;
    BranchHelper1Byte outer_loop;
    BranchHelper1Byte post_every_33_iterations;
    BranchHelper4Byte remainder_increment;
    BranchHelper4Byte thread_mainloop_continue;
    BranchHelper4Byte time_pass_loop_4byte;
    BranchHelper1Byte time_pass_loop_1byte;
    BranchHelper1Byte exit_remainder_increment;
    BranchHelper4Byte thread_mainloop;

    PageUnprotect pageEntry(logger, ThreadEntryPointAddress, ThreadEntryPointLength);
    PageUnprotect pageBody(logger, ThreadFunctionBodyAddress, ThreadFunctionBodyLength);

    {
        // this is the entry point of the function we're replacing, clear it out
        char* tmp = ThreadEntryPointAddress;
        for (size_t i = 0; i < ThreadEntryPointLength; ++i) {
            *tmp++ = static_cast<char>(0xcc);
        }
    }
    {
        // this is the body of the function we're replacing, clear it out
        // don't ask me why this is split up like this
        char* tmp = ThreadFunctionBodyAddress;
        for (size_t i = 0; i < ThreadFunctionBodyLength; ++i) {
            *tmp++ = static_cast<char>(0xcc);
        }
    }

    lock_mutex.SetTarget(LockMutex);
    unlock_mutex.SetTarget(UnlockMutex);
    invoke_sleep_milliseconds.SetTarget(InvokeSleepMilliseconds);
    process_sound_queue.SetTarget(ProcessSoundQueue);
    allmul.SetTarget(AllMul);
    alldvrm.SetTarget(AllDvRm);

    using JC = JumpCondition;
    // clang-format off
    {
        // BGM Timing: Thread Entry
        char* tmp = ThreadEntryPointAddress;
        WriteInstruction8(tmp, 0x55);            // push       ebp
        WriteInstruction16(tmp, 0x8bec);         // mov        ebp,esp
        WriteInstruction24(tmp, 0x83ec7c);       // sub        esp,7Ch
        WriteInstruction24(tmp, 0x8b4d08);       // mov        ecx,dword ptr [ebp+8]
        WriteInstruction8(tmp, 0x57);            // push       edi
        WriteInstruction8(tmp, 0x56);            // push       esi
        thread_mainloop.WriteJump(tmp, JC::JMP); // jmp        thread_mainloop
    }
    {
        // BGM Timing: Remainder Increment
        char*& tmp = codespace;
        remainder_increment.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8b45bc);                // mov        eax,dword ptr [ebp-44h]  ; load counter into eax
        WriteInstruction16(tmp, 0x03c2);                  // add        eax,edx                  ; counter += ticks_per_loop_remainder
        WriteInstruction24(tmp, 0x8b55c8);                // mov        edx,dword ptr [ebp-38h]  ; edx = original_divisor
        WriteInstruction16(tmp, 0x3bc2);                  // cmp        eax,edx                  ; if counter >= original_divisor
        exit_remainder_increment.WriteJump(tmp, JC::JB);  // jb         exit_remainder_increment
        WriteInstruction32(tmp, 0x8345f001);              // add        dword ptr [ebp-10h],1    ; ++ticks_last
        WriteInstruction32(tmp, 0x8355f400);              // adc        dword ptr [ebp-0Ch],0
        WriteInstruction16(tmp, 0x2bc2);                  // sub        eax,edx                  ; counter -= original_divisor
        exit_remainder_increment.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8945bc);                // mov        dword ptr [ebp-44h],eax  ; write counter back to stack
        time_pass_loop_4byte.WriteJump(tmp, JC::JMP);     // jmp        time_pass_loop
    }
    {
        // BGM Timing: Thread Init 1
        char*& tmp = codespace;
        thread_mainloop.SetTarget(tmp);
        WriteInstruction8(tmp, 0x53);                                // push       ebx
        WriteInstruction16(tmp, 0x8bf9);                             // mov        edi,ecx
        WriteInstruction16(tmp, 0x33f6);                             // xor        esi,esi
        WriteInstruction56(tmp, 0xf74710ffffff7f);                   // test       dword ptr [edi+10h],7FFFFFFFh
        early_exit.WriteJump(tmp, JC::JE);                           // je         early_exit
        WriteInstruction24(tmp, 0x8d45d8);                           // lea        eax,[ebp-28h]
        invoke_query_performance_frequency.WriteJump(tmp, JC::CALL); // call       invoke_query_performance_frequency
        thread_mainloop_continue.WriteJump(tmp, JC::JMP);            // jmp        thread_mainloop_continue
    }
    {
        // BGM Timing: Thread Init 2
        char*& tmp = codespace;
        thread_mainloop_continue.SetTarget(tmp);
        WriteInstruction8(tmp, 0xb8);                              // mov        eax,(divisor)
        std::memcpy(tmp, &divisor, 4);
        tmp += 4;
        WriteInstruction24(tmp, 0x8945c8);                         // mov        dword ptr [ebp-38h],eax
        WriteInstruction16(tmp, 0x6a00);                           // push       0
        WriteInstruction8(tmp, 0x50);                              // push       eax
        WriteInstruction24(tmp, 0x8b45dc);                         // mov        eax,dword ptr [ebp-24h]
        WriteInstruction24(tmp, 0x8b55d8);                         // mov        edx,dword ptr [ebp-28h]
        WriteInstruction8(tmp, 0x50);                              // push       eax
        WriteInstruction8(tmp, 0x52);                              // push       edx
        alldvrm.WriteJump(tmp, JC::CALL);                          // call       _alldvrm
        WriteInstruction24(tmp, 0x8945e0);                         // mov        dword ptr [ebp-20h],eax
        WriteInstruction24(tmp, 0x8955e4);                         // mov        dword ptr [ebp-1Ch],edx
        WriteInstruction24(tmp, 0x894dc0);                         // mov        dword ptr [ebp-40h],ecx

        WriteInstruction24(tmp, 0x8b45dc);                         // mov        eax,dword ptr [ebp-24h]
        WriteInstruction24(tmp, 0x8b55d8);                         // mov        edx,dword ptr [ebp-28h]
        WriteInstruction16(tmp, 0x6a00);                           // push       0
        WriteInstruction16(tmp, 0x6a05);                           // push       5
        WriteInstruction8(tmp, 0x50);                              // push       eax
        WriteInstruction8(tmp, 0x52);                              // push       edx
        allmul.WriteJump(tmp, JC::CALL);                           // call       _allmul
        WriteInstruction24(tmp, 0x8945d0);                         // mov        dword ptr [ebp-30h],eax
        WriteInstruction24(tmp, 0x8955d4);                         // mov        dword ptr [ebp-2Ch],edx

        WriteInstruction24(tmp, 0x8d45f0);                         // lea        eax,[ebp-10h]
        invoke_query_performance_counter.WriteJump(tmp, JC::CALL); // call       invoke_query_performance_counter
        WriteInstruction32(tmp, 0x807f5400);                       // cmp        byte ptr [edi+54h],0
        early_exit.WriteJump(tmp, JC::JNE);                        // jne        early_exit
        outer_loop_init.WriteJump(tmp, JC::JMP);                   // jmp        outer_loop_init
    }
    {
        // BGM Timing: Outer Loop
        char* tmp = ThreadFunctionBodyAddress;
        outer_loop_init.SetTarget(tmp);
        WriteInstruction56(tmp, 0xc745bc00000000);                 // mov        dword ptr [ebp-44h],0
        outer_loop.SetTarget(tmp);
        WriteInstruction16(tmp, 0x33db);                           // xor        ebx,ebx
        WriteInstruction24(tmp, 0x8d45e8);                         // lea        eax,[ebp-18h]
        invoke_query_performance_counter.WriteJump(tmp, JC::CALL); // call       invoke_query_performance_counter
        WriteInstruction24(tmp, 0x8d45e8);                         // lea        eax,[ebp-18h] ; ticks_now
        WriteInstruction24(tmp, 0x8d55d0);                         // lea        edx,[ebp-30h] ; ticks_for_reset
        WriteInstruction24(tmp, 0x8d4df0);                         // lea        ecx,[ebp-10h] ; ticks_last
        do_compare.WriteJump(tmp, JC::CALL);                       // call       do_compare
        WriteInstruction16(tmp, 0x85c0);                           // test       eax,eax
        time_pass_loop_1byte.WriteJump(tmp, JC::JE);               // je         time_pass_loop
        WriteInstruction24(tmp, 0x8b45e8);                         // mov        eax,dword ptr [ebp-18h]
        WriteInstruction24(tmp, 0x8945f0);                         // mov        dword ptr [ebp-10h],eax
        WriteInstruction24(tmp, 0x8b45ec);                         // mov        eax,dword ptr [ebp-14h]
        WriteInstruction24(tmp, 0x8945f4);                         // mov        dword ptr [ebp-0Ch],eax
        time_pass_loop_1byte.SetTarget(tmp);
        time_pass_loop_4byte.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8d45e8);                         // lea        eax,[ebp-18h] ; ticks_now
        WriteInstruction24(tmp, 0x8d55e0);                         // lea        edx,[ebp-20h] ; ticks_per_loop
        WriteInstruction24(tmp, 0x8d4df0);                         // lea        ecx,[ebp-10h] ; ticks_last
        do_compare.WriteJump(tmp, JC::CALL);                       // call       do_compare
        WriteInstruction16(tmp, 0x85c0);                           // test       eax,eax
        go_to_sleep_maybe.WriteJump(tmp, JC::JE);                  // je         go_to_sleep_maybe
        inner_loop.WriteJump(tmp, JC::JMP);                        // jmp        inner_loop
        exit_inner_loop.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8b45e0);                         // mov        eax,dword ptr [ebp-20h] ; ticks_per_loop
        WriteInstruction24(tmp, 0x8b4de4);                         // mov        ecx,dword ptr [ebp-1Ch]
        WriteInstruction24(tmp, 0x0145f0);                         // add        dword ptr [ebp-10h],eax ; ticks_last
        WriteInstruction24(tmp, 0x114df4);                         // adc        dword ptr [ebp-0Ch],ecx
        WriteInstruction24(tmp, 0x8b55c0);                         // mov        edx,dword ptr [ebp-40h] ; ticks_per_loop_remainder
        WriteInstruction16(tmp, 0x85d2);                           // test       edx,edx
        time_pass_loop_1byte.WriteJump(tmp, JC::JE);               // je         time_pass_loop ; no remainder, just go back to loop
        remainder_increment.WriteJump(tmp, JC::JMP);               // jmp        remainder_increment
        go_to_sleep_maybe.SetTarget(tmp);
        WriteInstruction16(tmp, 0x85db);                           // test       ebx,ebx
        go_to_next_iteration.WriteJump(tmp, JC::JNE);              // jne        go_to_next_iteration
        WriteInstruction16(tmp, 0x6a00);                           // push       0
        invoke_sleep_milliseconds.WriteJump(tmp, JC::CALL);        // call       invoke_sleep_milliseconds
        WriteInstruction24(tmp, 0x83c404);                         // add        esp,4
        go_to_next_iteration.SetTarget(tmp);
        WriteInstruction32(tmp, 0x807f5400);                       // cmp        byte ptr [edi+54h],0
        outer_loop.WriteJump(tmp, JC::JE);                         // je         outer_loop
        early_exit.SetTarget(tmp);
        WriteInstruction8(tmp, 0x5b);                              // pop        ebx
        WriteInstruction8(tmp, 0x5e);                              // pop        esi
        WriteInstruction16(tmp, 0x33c0);                           // xor        eax,eax
        WriteInstruction8(tmp, 0x5f);                              // pop        edi
        WriteInstruction16(tmp, 0x8be5);                           // mov        esp,ebp
        WriteInstruction8(tmp, 0x5d);                              // pop        ebp
        WriteInstruction8(tmp, 0xc3);                              // ret
    }
    {
        // BGM Timing: Inner Loop
        char*& tmp = codespace;
        inner_loop.SetTarget(tmp);
        WriteInstruction40(tmp, 0xbb01000000);                     // mov        ebx,1 ; remember that we've executed an inner_loop
        WriteInstruction24(tmp, 0x8d4f38);                         // lea        ecx,[edi+38h]
        lock_mutex.WriteJump(tmp, JC::CALL);                       // call       lock_mutex
        WriteInstruction16(tmp, 0x8bcf);                           // mov        ecx,edi
        process_sound_queue.WriteJump(tmp, JC::CALL);              // call       unknown_func
        WriteInstruction24(tmp, 0x83fe21);                         // cmp        esi,21h
        post_every_33_iterations.WriteJump(tmp, JC::JB);           // jb         post_every_33_iterations
        WriteInstruction32(tmp, 0x660f6ec6);                       // movd       xmm0,esi
        WriteInstruction24(tmp, 0x0f5bc0);                         // cvtdq2ps   xmm0,xmm0
        WriteInstruction16(tmp, 0x8b17);                           // mov        edx,dword ptr [edi]
        WriteInstruction8(tmp, 0x51);                              // push       ecx
        WriteInstruction16(tmp, 0x8bcf);                           // mov        ecx,edi
        WriteInstruction32(tmp, 0xf30f5e05);                       // divss      xmm0,dword ptr ds:[8ED254h]
        std::memcpy(tmp, &InnerLoopDivss, 4);
        tmp += 4;
        WriteInstruction40(tmp, 0xf30f594758);                     // mulss      xmm0,dword ptr [edi+58h]
        WriteInstruction40(tmp, 0xf30f110424);                     // movss      dword ptr [esp],xmm0
        WriteInstruction24(tmp, 0xff5268);                         // call       dword ptr [edx+68h]
        WriteInstruction24(tmp, 0x83ee21);                         // sub        esi,21h
        post_every_33_iterations.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8d4f38);                         // lea        ecx,[edi+38h]
        unlock_mutex.WriteJump(tmp, JC::CALL);                     // call       unlock_mutex
        WriteInstruction8(tmp, 0x46);                              // inc        esi
        exit_inner_loop.WriteJump(tmp, JC::JMP);                   // jmp        exit_inner_loop
    }
    {
        // BGM Timing: QueryPerformanceFrequency
        char*& tmp = codespace;
        invoke_query_performance_frequency.SetTarget(tmp);
        WriteInstruction8(tmp, 0x50);                              // push eax
        WriteInstruction16(tmp, 0xff15);                           // call dword ptr[QueryPerformanceFrequency]
        std::memcpy(tmp, &QueryPerformanceFrequency, 4);
        tmp += 4;
        WriteInstruction8(tmp, 0xc3);                              // ret
    }
    {
        // BGM Timing: QueryPerformanceCounter
        char*& tmp = codespace;
        invoke_query_performance_counter.SetTarget(tmp);
        WriteInstruction8(tmp, 0x50);                              // push eax
        WriteInstruction16(tmp, 0xff15);                           // call dword ptr[QueryPerformanceCounter]
        std::memcpy(tmp, &QueryPerformanceCounter, 4);
        tmp += 4;
        WriteInstruction8(tmp, 0xc3);                              // ret
    }
    {
        // BGM Timing: do_compare
        char*& tmp = codespace;
        do_compare.SetTarget(tmp);
        WriteInstruction8(tmp, 0x53);                              // push        ebx
        WriteInstruction16(tmp, 0x8bd8);                           // mov         ebx,eax
        WriteInstruction24(tmp, 0x8b4104);                         // mov         eax,dword ptr [ecx+4]
        WriteInstruction8(tmp, 0x56);                              // push        esi
        WriteInstruction16(tmp, 0x8b31);                           // mov         esi,dword ptr [ecx]
        WriteInstruction16(tmp, 0x0332);                           // add         esi,dword ptr [edx]
        WriteInstruction16(tmp, 0x8bcb);                           // mov         ecx,ebx
        WriteInstruction24(tmp, 0x134204);                         // adc         eax,dword ptr [edx+4]
        WriteInstruction24(tmp, 0x3b4104);                         // cmp         eax,dword ptr [ecx+4]
        success.WriteJump(tmp, JC::JG);                            // jg          success
        fail.WriteJump(tmp, JC::JL);                               // jl          fail
        WriteInstruction16(tmp, 0x3b31);                           // cmp         esi,dword ptr [ecx]
        success.WriteJump(tmp, JC::JAE);                           // jae         success
        fail.SetTarget(tmp);
        WriteInstruction40(tmp, 0xb801000000);                     // mov         eax,1
        do_compare_end.WriteJump(tmp, JC::JMP);                    // jmp         do_compare_end
        success.SetTarget(tmp);
        WriteInstruction16(tmp, 0x33c0);                           // xor         eax,eax
        do_compare_end.SetTarget(tmp);
        WriteInstruction8(tmp, 0x5e);                              // pop         esi
        WriteInstruction8(tmp, 0x5b);                              // pop         ebx
        WriteInstruction8(tmp, 0xc3);                              // ret
    }
    // clang-format on
}
} // namespace SenLib::Sen2
