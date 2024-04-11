#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchMusicQueueing(SenPatcher::Logger& logger,
                        char* textRegion,
                        GameVersion version,
                        char*& codespace,
                        char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const bgmPlayingCheckInjectAddress =
        GetCodeAddressJpEn(version, textRegion, 0x140310f72, 0x14031a377);

    // This behaves similarly but somewhat different to the CS2 music stuff. As far as I can tell,
    // the basic gist is the same, but for the music commands are passed outside of the standard
    // audio queue.
    //
    // A few notable addresses:
    // 0x1401176af -> Sets the music buffer to 'fading' (1). The line above is the time of the fade,
    // in seconds. This is set on the *main thread*, not the sound thread (unlike CS2!).
    //
    // 0x1401184da -> Sets the music buffer to 'fading done' (2, maybe 'stopped'). This is set on
    // the sound thread.
    //
    // 0x140111536 -> Sets the music buffer to 'playing' (0) for the new track. This is set on the
    // sound thread.
    //
    // 0x1401107a0 called at 0x14031a371 (main thread) appears to return 0 if the current music
    // track is different to the new one, 1 if it's the same one, and 2 if it's the same one but
    // currently fading. The caller checks for == 0 and only continues the music enqueueing if that
    // is true.
    //
    // With this, I think there's actually a pretty easy fix here: Check for != 1 instead of == 0.
    // Since the music track is set to fading on the main thread, there should be no race condition.
    // - If the to-enqueue music track is different from the playing one, the function will return
    // 0. Continue the enqueue.
    // - If the to-enqueue music track is still playing and no fade is active, the function will
    // return 1. Skip the enqueue.
    // - If the to-enqueue music track is currently fading (even if the fade hasn't started yet, it
    // just has been set to fade by the main thread) the function will return 2. Continue the
    // enqueue.
    // - If the to-enqueue music track is done fading but the new track hasn't started yet, the
    // function will still return 2. Continue the enqueue.
    //
    // The worst that can happen here is that we enqueue a track that is already enqueued but hasn't
    // started playing yet, which should be fairly harmless.
    //
    // This makes me think that Falcom was aware of this bug and tried to fix it, but messed up the
    // last step somehow. Either way, easier for us...

    // We're injecting over:
    // test eax,eax
    // jz [continueEnqueueTargetAddress]
    // cmp byte ptr[rsp+0x4f0],0
    // jnz [continueEnqueueTargetAddress]
    //
    // we're essentially just writing another version of that block but instead of
    // checking eax for == 0 we check for != 1
    char*& tmp = codespace;
    const auto injectResult = InjectJumpIntoCode<14, PaddingInstruction::Nop>(
        logger, bgmPlayingCheckInjectAddress, R64::RCX, codespace);
    const char* const continueEnqueueTargetAddress =
        bgmPlayingCheckInjectAddress + 4
        + static_cast<int8_t>(injectResult.OverwrittenInstructions[3]);

    WriteInstruction24(tmp, 0x83f801); // cmp eax,1
    BranchHelper1Byte continue_enqueue;
    continue_enqueue.WriteJump(tmp, JumpCondition::JNE);

    // extra check whether to continue enqueueing anyway (there's a bool parameter to the function
    // that's effectively 'enqueue always, regardless of track playing state', which is checked
    // after the expensive check for some reason...)
    std::memcpy(tmp,
                injectResult.OverwrittenInstructions.data() + 4,
                injectResult.OverwrittenInstructions.size() - 6);
    tmp += (injectResult.OverwrittenInstructions.size() - 6);
    continue_enqueue.WriteJump(tmp, JumpCondition::JNE);

    // skip the enqueue
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(codespace, R64::RCX);

    // continue the enqueue
    continue_enqueue.SetTarget(tmp);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(continueEnqueueTargetAddress));
    Emit_JMP_R64(codespace, R64::RCX);
}
} // namespace SenLib::Sen3
