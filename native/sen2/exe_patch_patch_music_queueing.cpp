#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchMusicQueueingOnSoundThreadSide(SenPatcher::Logger& logger,
                                         char* textRegion,
                                         GameVersion version,
                                         char*& codespace,
                                         char* codespaceEnd) {
    using namespace SenPatcher::x86;

    char* const LockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d4f0, 0x71e550);
    char* const UnlockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d520, 0x71e580);
    char* const BgmAlreadyPlayingJump = GetCodeAddressJpEn(version, textRegion, 0x57c49d, 0x57c80d);
    char* const AddressOfSkipEnqueueOnSoundThreadSideInjection =
        GetCodeAddressJpEn(version, textRegion, 0x41ef17, 0x41ef37);

    // clang-format off
    BranchHelper4Byte lock_mutex;
    BranchHelper4Byte unlock_mutex;
    BranchHelper4Byte inject_entry_point;
    BranchHelper1Byte enqueue_check;
    BranchHelper1Byte exit_continue;
    BranchHelper1Byte exit_skip;
    BranchHelper4Byte back_to_function;

    lock_mutex.SetTarget(LockMutex);
    unlock_mutex.SetTarget(UnlockMutex);

    using JC = JumpCondition;

    // ignore the check if the track is already playing on main thread side
    {
        char* tmp = BgmAlreadyPlayingJump;
        PageUnprotect page(logger, tmp, 1);
        WriteInstruction8(tmp, 0xeb);

        // for testing, write command even if the tracked track is the same
        //tmp = GetCodeAddressJpEn(version, textRegion, 0, 0x57c836);
        //WriteInstruction8(tmp, 0x90);
        //WriteInstruction8(tmp, 0x90);
    }

    // on audio thread side, inject after the bgm play command is extracted from the ringbuffer
    // but before it is actually used to switch to the BGM
    {
        char* tmp = AddressOfSkipEnqueueOnSoundThreadSideInjection;
        PageUnprotect page(logger, tmp, 5);
        inject_entry_point.SetTarget(codespace);
        inject_entry_point.WriteJump(tmp, JC::JMP);
        back_to_function.SetTarget(tmp);
    }
    {
        char*& tmp = codespace;
        WriteInstruction56(tmp, 0xf74610ffffff7f);   // test dword ptr[esi+10h],7FFFFFFFh
        enqueue_check.WriteJump(tmp, JC::JNE);       // jne  enqueue_check
        WriteInstruction16(tmp, 0x8bce);             // mov  ecx,esi
        WriteInstruction24(tmp, 0xff5034);           // call dword ptr[eax+34h]
        back_to_function.WriteJump(tmp, JC::JMP);    // jmp  back_to_function

        enqueue_check.SetTarget(tmp);
        WriteInstruction8(tmp, 0x57);                // push edi
        WriteInstruction24(tmp, 0x8d4e40);           // lea  ecx,[esi+40h]
        lock_mutex.WriteJump(tmp, JC::CALL);         // call lock_mutex
        WriteInstruction24(tmp, 0x8b4614);           // mov  eax,dword ptr[esi+14h]
        WriteInstruction32(tmp, 0x8b4c241c);         // mov  ecx,dword ptr[esp+1Ch]
        WriteInstruction24(tmp, 0x8b3c88);           // mov  edi,dword ptr[eax+ecx*4h]

        WriteInstruction16(tmp, 0x8b07);             // mov  eax,dword ptr[edi]
        WriteInstruction16(tmp, 0x8bcf);             // mov  ecx,edi
        WriteInstruction24(tmp, 0x8b4010);           // mov  eax,dword ptr[eax+10h]
        WriteInstruction16(tmp, 0xffd0);             // call eax
        WriteInstruction16(tmp, 0x84c0);             // test al,al
        exit_continue.WriteJump(tmp, JC::JE);        // je   exit_continue

        WriteInstruction24(tmp, 0x8b4708);           // mov  eax,dword ptr[edi+8h]
        WriteInstruction16(tmp, 0x85c0);             // test eax,eax
        exit_continue.WriteJump(tmp, JC::JE);        // je   exit_continue
        WriteInstruction24(tmp, 0x8b404c);           // mov  eax,dword ptr[eax+4Ch] 
        WriteInstruction48(tmp, 0x8b8dccfeffff);     // mov  ecx,dword ptr[ebp-134h]
        WriteInstruction16(tmp, 0x3bc1);             // cmp  eax,ecx
        exit_continue.WriteJump(tmp, JC::JNE);       // jne  exit_continue

        WriteInstruction24(tmp, 0x8a4738);           // mov  al,byte ptr[edi+38h]
        WriteInstruction16(tmp, 0x84c0);             // test al,al
        exit_continue.WriteJump(tmp, JC::JNE);       // jne  exit_continue

        WriteInstruction8(tmp, 0x5f);                // pop  edi
        WriteInstruction24(tmp, 0x83c41c);           // add  esp,1Ch
        exit_skip.WriteJump(tmp, JC::JMP);           // jmp  exit_skip

        exit_continue.SetTarget(tmp);
        WriteInstruction8(tmp, 0x5f);                // pop  edi
        WriteInstruction16(tmp, 0x8b06);             // mov  eax,dword ptr[esi]
        WriteInstruction16(tmp, 0x8bce);             // mov  ecx,esi
        WriteInstruction24(tmp, 0xff5034);           // call dword ptr[eax+34h]

        exit_skip.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8d4e40);           // lea  ecx,[esi+40h]
        unlock_mutex.WriteJump(tmp, JC::CALL);       // call unlock_mutex
        back_to_function.WriteJump(tmp, JC::JMP);    // jmp  back_to_function
    }
    // clang-format on
}
} // namespace SenLib::Sen2
