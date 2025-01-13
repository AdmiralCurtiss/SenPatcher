#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

// NOTE: When calling a function, stack must be aligned to 0x10 bytes!

namespace SenLib::Sen4 {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version,
                                                textRegion,
                                                Addresses{.Jp121 = 0x1400ed273,
                                                          .En121 = 0x1400ef333,
                                                          .Jp122 = 0x1400ed1d3,
                                                          .En122 = 0x1400ef293});
    char* const exitPoint = GetCodeAddressJpEn(version,
                                               textRegion,
                                               Addresses{.Jp121 = 0x1400ed2d6,
                                                         .En121 = 0x1400ef396,
                                                         .Jp122 = 0x1400ed236,
                                                         .En122 = 0x1400ef2f6});


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

    char* const entryPoint = GetCodeAddressJpEn(version,
                                                textRegion,
                                                Addresses{.Jp121 = 0x1400ed1b0,
                                                          .En121 = 0x1400ef270,
                                                          .Jp122 = 0x1400ed110,
                                                          .En122 = 0x1400ef1d0});
    char* const exitPoint = GetCodeAddressJpEn(version,
                                               textRegion,
                                               Addresses{.Jp121 = 0x1400ed1ec,
                                                         .En121 = 0x1400ef2ac,
                                                         .Jp122 = 0x1400ed14c,
                                                         .En122 = 0x1400ef20c});


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

    char* const entryPoint = GetCodeAddressJpEn(version,
                                                textRegion,
                                                Addresses{.Jp121 = 0x14080ceb0,
                                                          .En121 = 0x14080f430,
                                                          .Jp122 = 0x14080ce10,
                                                          .En122 = 0x14080f390});


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
    WriteInstruction40(codespace, 0x4c8d442420); // lea r8,[rsp+20h]

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(fsoundOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte no_injected_file;
    no_injected_file.WriteJump(codespace, JumpCondition::JS);

    // if we have an injected file there's still two options
    BranchHelper1Byte common_return;
    BranchHelper1Byte failure;
    failure.WriteJump(codespace, JumpCondition::JZ);

    // opening file succeeded, load return value into RAX and return
    WriteInstruction40(codespace, 0x488b442420); // mov rax,[rsp+20h]
    common_return.WriteJump(codespace, JumpCondition::JMP);

    // opening file failed, return nullptr
    failure.SetTarget(codespace);
    Emit_XOR_R64_R64(codespace, R64::RAX, R64::RAX);
    common_return.SetTarget(codespace);
    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x28);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);
    Emit_RET(codespace);

    // if we have no injected file proceed with normal function
    no_injected_file.SetTarget(codespace);
    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x28);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen4
