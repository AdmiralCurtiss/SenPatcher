#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchMusicQueueingOnSoundThreadSide(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const LockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d4f0, 0x71e550);
    char* const UnlockMutex = GetCodeAddressJpEn(version, textRegion, 0x71d520, 0x71e580);
    char* const BgmAlreadyPlayingJump = GetCodeAddressJpEn(version, textRegion, 0x57c49d, 0x57c80d);
    char* const AddressOfSkipEnqueueOnSoundThreadSideInjection =
        GetCodeAddressJpEn(version, textRegion, 0x41ef17, 0x41ef37);

    // clang-format off
    BranchHelper4Byte lock_mutex;
    BranchHelper4Byte unlock_mutex;
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
    const auto injectResult = InjectJumpIntoCode<5>(logger, AddressOfSkipEnqueueOnSoundThreadSideInjection, codespace);
    {
        back_to_function.SetTarget(injectResult.JumpBackAddress);

        char* tmp = codespace;

        // not entirely sure what this is checking but all the audio code does this before accessing any data, so...
        WriteInstruction56(tmp, 0xf74610ffffff7f);   // test dword ptr[esi+10h],7FFFFFFFh
        enqueue_check.WriteJump(tmp, JC::JNE);       // jne  enqueue_check

        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();
        back_to_function.WriteJump(tmp, JC::JMP);    // jmp  back_to_function

        enqueue_check.SetTarget(tmp);

        // lock the bgm state mutex
        WriteInstruction8(tmp, 0x57);                // push edi
        WriteInstruction24(tmp, 0x8d4e40);           // lea  ecx,[esi+40h]
        lock_mutex.WriteJump(tmp, JC::CALL);         // call lock_mutex

        WriteInstruction24(tmp, 0x8b4614);           // mov  eax,dword ptr[esi+14h]
        // eax now points at FSoundChannelController ptr array
        WriteInstruction32(tmp, 0x8b4c241c);         // mov  ecx,dword ptr[esp+1Ch]
        // ecx now contains the channel number we were instructed to enqueue into
        WriteInstruction24(tmp, 0x8b3c88);           // mov  edi,dword ptr[eax+ecx*4h]
        // edi now points at FSoundChannelController of the bgm to enqueue

        // call virtual function to check if channel is active
        WriteInstruction16(tmp, 0x8b07);             // mov  eax,dword ptr[edi]
        WriteInstruction16(tmp, 0x8bcf);             // mov  ecx,edi
        WriteInstruction24(tmp, 0x8b4010);           // mov  eax,dword ptr[eax+10h]
        WriteInstruction16(tmp, 0xffd0);             // call eax
        // al now contains whether the channel is active
        WriteInstruction16(tmp, 0x84c0);             // test al,al
        // if channel is not active, continue the enqueue
        exit_continue.WriteJump(tmp, JC::JE);        // je   exit_continue

        // go to FSoundData of the current FSoundChannelController
        WriteInstruction24(tmp, 0x8b4708);           // mov  eax,dword ptr[edi+8h]
        // check for nullptr
        WriteInstruction16(tmp, 0x85c0);             // test eax,eax
        // if no FSoundData continue the enqueue
        exit_continue.WriteJump(tmp, JC::JE);        // je   exit_continue
        WriteInstruction24(tmp, 0x8b404c);           // mov  eax,dword ptr[eax+4Ch] 
        // eax now contains the id of the track currently playing
        WriteInstruction48(tmp, 0x8b8dccfeffff);     // mov  ecx,dword ptr[ebp-134h]
        // ecx now contains the id of the track we want to enqueue
        WriteInstruction16(tmp, 0x3bc1);             // cmp  eax,ecx
        // if tracks mismatch we always want to enqueue
        exit_continue.WriteJump(tmp, JC::JNE);       // jne  exit_continue

        // if we reach here the track playing and the track to enqueue are the same
        // see if it's currently fading out, because if so we want to enqueue anyway
        WriteInstruction24(tmp, 0x8a4738);           // mov  al,byte ptr[edi+38h]
        // al is now 1 when fade is active, 0 if not
        WriteInstruction16(tmp, 0x84c0);             // test al,al
        // if fading we do want to enqueue
        exit_continue.WriteJump(tmp, JC::JNE);       // jne  exit_continue

        // if we reach here we want to skip the enqueueing
        // fixup stack as if the function was called and leave
        WriteInstruction8(tmp, 0x5f);                // pop  edi
        WriteInstruction24(tmp, 0x83c41c);           // add  esp,1Ch
        exit_skip.WriteJump(tmp, JC::JMP);           // jmp  exit_skip

        exit_continue.SetTarget(tmp);

        // call virtual function to enqueue the track, parameters have already been set up before the injected ode
        // restore stack
        WriteInstruction8(tmp, 0x5f);                // pop  edi

        WriteInstruction16(tmp, 0x8b06);             // mov  eax,dword ptr[esi]
        // mov ecx,esi; call dword ptr[eax+34h]
        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        exit_skip.SetTarget(tmp);
        WriteInstruction24(tmp, 0x8d4e40);           // lea  ecx,[esi+40h]
        unlock_mutex.WriteJump(tmp, JC::CALL);       // call unlock_mutex
        back_to_function.WriteJump(tmp, JC::JMP);    // jmp  back_to_function

        codespace = tmp;
    }
    // clang-format on

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen2
