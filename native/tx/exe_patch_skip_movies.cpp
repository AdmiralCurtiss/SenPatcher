#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
static int32_t __fastcall CheckLogoMovie(const char* movieName, uint32_t skipAllMovies) {
    if (skipAllMovies != 0 || strcmp("tx_logo", movieName) == 0
        || strcmp("tx_logo_aksys", movieName) == 0) {
        return 0;
    }
    return -1;
}

void PatchSkipMovies(PatchExecData& execData, bool skipLogos, bool skipAllMovies) {
    if (!skipLogos && !skipAllMovies) {
        return;
    }

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;
    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x6011ff, 0x5ff67f);
    char* const exitPoint = GetCodeAddressSteamGog(version, textRegion, 0x6012c7, 0x5ff747);

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte check_logo_movie;
    void* checkLogoMovieFunc = CheckLogoMovie;
    check_logo_movie.SetTarget(static_cast<char*>(checkLogoMovieFunc));

    Emit_MOV_R32_R32(codespace, R32::ECX, R32::EDI);
    Emit_MOV_R32_IMM32(codespace, R32::EDX, skipAllMovies ? 1u : 0u);
    check_logo_movie.WriteJump(codespace, JumpCondition::CALL);

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // no skip, continue normally
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // want to skip, go to exit point
    success.SetTarget(codespace);
    Emit_ADD_R32_IMM32(codespace, R32::ESP, 0x18); // fixup stack
    BranchHelper4Byte success_exit;
    success_exit.SetTarget(exitPoint);
    success_exit.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
