#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"

namespace SenLib::Sen1 {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x4825ea, 0x483e5a);
    char* const exitPoint = GetCodeAddressJpEn(version, textRegion, 0x48262e, 0x483e9e);

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_open_forwarder;
    ffile_open_forwarder.SetTarget(static_cast<char*>(ffileOpenForwarder));

    // call forwarder
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::EBP, 8);
    Emit_MOV_R32_R32(codespace, R32::ECX, R32::ESI);
    ffile_open_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // on success go to return code
    success.SetTarget(codespace);
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

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x482780, 0x483ff0);

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
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // on success just return immediately
    success.SetTarget(codespace);
    Emit_RET_IMM16(codespace, 12);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen1
