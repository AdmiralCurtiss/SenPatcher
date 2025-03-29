#include "exe_patch.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

// #define HOOK_MAGIC_DESCRIPTION_GENERATOR
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace SenLib::Sen5 {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
static void HookMagicDescriptionGenerator(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPointSystemMenu = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x1403fd5aa, .En115 = 0x1403fad6a});
    char* const entryPointDivineKnightBattle = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x1400c881f, .En115 = 0x1400c8a9f});

    void* handle = CreateFileMappingW(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 0x1000, L"SenMagicDescGenHook");
    if (handle == nullptr) {
        return;
    }

    void* memory = MapViewOfFileEx(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000, nullptr);
    if (memory == nullptr) {
        return;
    }

    {
        auto injectResult =
            InjectJumpIntoCode<13>(logger, entryPointSystemMenu, R64::RAX, codespace);
        char* inject = injectResult.JumpBackAddress;
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(memory));
        WriteInstruction16(codespace, 0x8b18); // mov ebx,dword ptr[rax]
        Emit_TEST_R64_R64(codespace, R64::RBX, R64::RBX);
        BranchHelper1Byte branch;
        branch.WriteJump(codespace, JumpCondition::JZ);

        WriteInstruction24(codespace, 0x8b5004); // mov edx,dword ptr[rax+4]

        branch.SetTarget(codespace);

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
        Emit_JMP_R64(codespace, R64::RAX);
    }
    {
        auto injectResult =
            InjectJumpIntoCode<14>(logger, entryPointDivineKnightBattle, R64::RCX, codespace);
        char* inject = injectResult.JumpBackAddress;
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(memory));
        WriteInstruction16(codespace, 0x8b10); // mov edx,dword ptr[rax]
        Emit_TEST_R64_R64(codespace, R64::RDX, R64::RDX);
        BranchHelper1Byte keep;
        keep.WriteJump(codespace, JumpCondition::JZ);

        WriteInstruction24(codespace, 0x8b5004); // mov edx,dword ptr[rax+4]

        BranchHelper1Byte overwrite;
        overwrite.WriteJump(codespace, JumpCondition::JMP);

        keep.SetTarget(codespace);
        overwrite.SetTarget(codespace + 3);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    execData.Codespace = codespace;
}
#endif

void PatchBrokenSprintf(PatchExecData& execData) {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
    HookMagicDescriptionGenerator(execData);
#endif

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
