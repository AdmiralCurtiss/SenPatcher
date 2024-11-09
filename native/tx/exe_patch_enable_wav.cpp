#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
using PFFileGetFilesizeForwarder = int32_t(__fastcall*)(const char* path, uint32_t* out_filesize);

// by returning the given path we don't have to do any cleanup on the call site,
// since it will be put into eax
static char* __fastcall CheckWavThenOgg(PFFileGetFilesizeForwarder ffileGetFilesizeForwarder,
                                        char* path) {
    char* p = path;
    size_t buffer_length = 0x100;
    if (p[0] == '.' && p[1] == '/') {
        p += 2;
        buffer_length -= 2;
    }

    size_t string_length = strnlen(p, buffer_length);
    if (string_length < 4 || string_length >= buffer_length) {
        return path;
    }

    // check for .wav
    p[string_length - 3] = 'w';
    p[string_length - 2] = 'a';
    p[string_length - 1] = 'v';
    p[string_length - 0] = '\0';
    if (ffileGetFilesizeForwarder(p, nullptr) == 1) {
        // .wav exists!
        return path;
    }

    // switch back to .ogg
    p[string_length - 3] = 'o';
    p[string_length - 2] = 'g';
    p[string_length - 1] = 'g';
    p[string_length - 0] = '\0';
    return path;
}

void PatchEnableWav(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    // FIXME: This is sloppy, we trigger a lookup every time a sound effect is played, even when
    // it's in the cache.

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;

    char* const addressSE = GetCodeAddressSteamGog(version, textRegion, 0x4dde5e, 0x4dc87e);
    char* const addressBGM = GetCodeAddressSteamGog(version, textRegion, 0x4dde19, 0x4dc839);

    char* codespace = execData.Codespace;

    BranchHelper4Byte check_wav_then_ogg;
    void* check_wav_then_ogg_ptr = CheckWavThenOgg;
    check_wav_then_ogg.SetTarget(static_cast<char*>(check_wav_then_ogg_ptr));

    {
        auto injectResult = InjectJumpIntoCode<5>(logger, addressSE, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        Emit_MOV_R32_IMM32(codespace, R32::ECX, std::bit_cast<uint32_t>(ffileGetFilesizeForwarder));
        Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
        check_wav_then_ogg.WriteJump(codespace, JumpCondition::CALL);

        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    {
        auto injectResult = InjectJumpIntoCode<5>(logger, addressBGM, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        Emit_MOV_R32_IMM32(codespace, R32::ECX, std::bit_cast<uint32_t>(ffileGetFilesizeForwarder));
        Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
        check_wav_then_ogg.WriteJump(codespace, JumpCondition::CALL);

        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
