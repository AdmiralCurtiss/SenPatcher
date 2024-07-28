#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"

namespace SenLib::TX {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x44b964, 0x44a2b4);
    char* const exitPoint = GetCodeAddressSteamGog(version, textRegion, 0x44b95b, 0x44a2ab);

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_open_forwarder;
    ffile_open_forwarder.SetTarget(static_cast<char*>(ffileOpenForwarder));

    // call forwarder
    Emit_MOV_R32_R32(codespace, R32::EDX, R32::EDI);
    Emit_MOV_R32_R32(codespace, R32::ECX, R32::ESI);
    ffile_open_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file go to return code, return value is already correct in EAX
    success.SetTarget(codespace);
    Emit_ADD_R32_IMM32(codespace, R32::ESP, 8); // fixup stack
    BranchHelper4Byte success_exit;
    success_exit.SetTarget(exitPoint);
    success_exit.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}

void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x44b820, 0x44a170);

    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_get_size_forwarder;
    ffile_get_size_forwarder.SetTarget(static_cast<char*>(ffileGetFilesizeForwarder));

    // call forwarder
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::ECX, R32::ESP, 4);
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::ESP, 12);
    ffile_get_size_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file just return immediately, return value is already correct in EAX
    success.SetTarget(codespace);
    Emit_RET_IMM16(codespace, 12);

    execData.Codespace = codespace;
}

/*
void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x41ae91, 0x41aec1);
    char* const compressionFlagCheck1 =
        GetCodeAddressSteamGog(version, textRegion, 0x41add8, 0x41ae08);
    char* const compressionFlagCheck2 =
        GetCodeAddressSteamGog(version, textRegion, 0x404b0a, 0x404b1a);

    // TODO: Is that all the '(flags & 1) != 0' checks?

    {
        // this changes a '(flags & 1) != 0' check to a '(flags & 0xfd) != 0' check
        // that way compression is detected if any non-checksum flag is set, and not just bit 0
        char* tmp = compressionFlagCheck1;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0xfd;
    }
    {
        char* tmp = compressionFlagCheck2;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0xfd;
    }

    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode<7, PaddingInstruction::Nop>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);

    BranchHelper4Byte decompress_pkg_forwarder;
    decompress_pkg_forwarder.SetTarget(static_cast<char*>(decompressPkgForwarder));

    // call forwarder
    Emit_MOV_R32_R32(codespace, R32::ECX, R32::ESI);
    Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
    Emit_PUSH_R32(codespace, R32::EDI);
    decompress_pkg_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // fix stack
    Emit_SUB_R32_IMM32(codespace, R32::ESP, 0x8);

    // no return value to check, so just go back
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}
*/
} // namespace SenLib::TX
