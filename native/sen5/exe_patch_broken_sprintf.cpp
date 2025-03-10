#include "exe_patch.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

namespace SenLib::Sen5 {
void PatchBrokenSprintf(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    // In the magic description generator there's a call that's something like:
    // snprintf_s(buffer, length, string_from_t_itemhelp, str0, str1, str2, str3);
    // This can crash the game with a 'An invalid parameter was passed to a function that considers
    // invalid parameters fatal.' exception, though I'm not really sure why if I'm being honest...
    // (In the vanilla game this doesn't happen because all magic descriptions are explicit and the
    // generator is not invoked -- that's the Z flag in the magic description flags, see t_magic.)
    // Replacing this with snprintf() makes the construct work as intended.

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x1403f609f, .En115 = 0x1403f381f});


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    std::memcpy(codespace, overwrittenInstructions.data(), 7);
    codespace += 7;

    void* func = snprintf;
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(func));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen5
