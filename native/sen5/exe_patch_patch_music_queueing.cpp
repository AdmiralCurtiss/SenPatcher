#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchMusicQueueing(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const bgmPlayingCheckInjectAddress = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x1403438cc, .En115 = 0x140342ddc});

    // Exact same fix as CS3, see that for details.
    //
    // Addresses: (v1.1.4)
    // 0x1400a3faf -> Sets the music buffer to 'fading' (1). Main thread.
    // 0x1400bcf01 -> Sets the music buffer to 'fading done' (2). Sound thread.
    // 0x1400ac9bf -> Sets the music buffer to 'playing' (0) for the new track. Sound thread. (can
    // also be on the main thread if no track is playing at all right now)
    //
    // 0x1400acbe0 called at 0x1403438ca is the state check function.

    // We're injecting over:
    // test eax,eax
    // jz [continueEnqueueTargetAddress]
    // cmp byte ptr[rsp+0x4d0],r13b
    // jnz [continueEnqueueTargetAddress]
    //
    // we're essentially just writing another version of that block but instead of
    // checking eax for == 0 we check for != 1
    const auto injectResult = InjectJumpIntoCode<14, PaddingInstruction::Nop>(
        logger, bgmPlayingCheckInjectAddress, R64::RCX, codespace);
    const char* const continueEnqueueTargetAddress =
        bgmPlayingCheckInjectAddress + 4
        + static_cast<int8_t>(injectResult.OverwrittenInstructions[3]);

    WriteInstruction24(codespace, 0x83f801); // cmp eax,1
    BranchHelper1Byte continue_enqueue;
    continue_enqueue.WriteJump(codespace, JumpCondition::JNE);

    // extra check whether to continue enqueueing anyway (there's a bool parameter to the function
    // that's effectively 'enqueue always, regardless of track playing state', which is checked
    // after the expensive check for some reason...)
    std::memcpy(codespace,
                injectResult.OverwrittenInstructions.data() + 4,
                injectResult.OverwrittenInstructions.size() - 6);
    codespace += (injectResult.OverwrittenInstructions.size() - 6);
    continue_enqueue.WriteJump(codespace, JumpCondition::JNE);

    // skip the enqueue
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(codespace, R64::RCX);

    // continue the enqueue
    continue_enqueue.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(continueEnqueueTargetAddress));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen5
