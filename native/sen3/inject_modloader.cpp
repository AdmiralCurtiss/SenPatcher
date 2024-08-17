#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

// NOTE: When calling a function, stack must be aligned to 0x10 bytes!

namespace SenLib::Sen3 {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x1401316f3, 0x140134e83);
    char* const exitPoint = GetCodeAddressJpEn(version, textRegion, 0x140131756, 0x140134ee6);


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RDX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RDI);
    Emit_MOV_R64_R64(codespace, R64::RCX, R64::RBX);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RDX);

    // if we have an injected file go to return code, return value is already correct in RAX
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RDX);

    execData.Codespace = codespace;
}

void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x140131630, 0x140134dc0);
    char* const exitPoint = GetCodeAddressJpEn(version, textRegion, 0x14013166c, 0x140134dfc);


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<13>(logger, entryPoint, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);
    Emit_PUSH_R64(codespace, R64::R9);
    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_R64(codespace, R64::RDX, R64::R9);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RDX);

    // if we have an injected file go to return code, return value is already correct in RAX
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}

void InjectAtOpenFSoundFile(PatchExecData& execData, void* fsoundOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x140086900, 0x140086900);


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<15>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);
    Emit_PUSH_R64(codespace, R64::R9);
    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x28);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(fsoundOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x28);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure go back to function
    Emit_MOV_R64_IMM64(codespace, R64::R9, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::R9);

    // on success just return from the function, RAX already has the correct return value
    success.SetTarget(codespace);
    Emit_RET(codespace);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen3
