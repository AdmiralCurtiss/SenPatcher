#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

// NOTE: When calling a function, stack must be aligned to 0x10 bytes!

namespace SenLib::Sen5 {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140088732, .En115 = 0x140088e22});
    char* const exitPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140088780, .En115 = 0x140088e70});


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RDX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_PUSH_R64(codespace, R64::R10);
    Emit_PUSH_R64(codespace, R64::R11);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RBP);
    Emit_MOV_R64_R64(codespace, R64::RCX, R64::RSI);
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
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // if we have an injected file go to exit point, and write return value to RBX
    success.SetTarget(codespace);
    Emit_MOV_R64_R64(codespace, R64::RBX, R64::RAX);
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RDX);

    execData.Codespace = codespace;
}

void InjectAtBattleScriptExists(PatchExecData& execData, void* ffileExistsForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14014844a, .En115 = 0x140147cba});
    char* const exitPointOnSuccess = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140148461, .En115 = 0x140147cd1});
    char* const paddingSpace = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140149de4, .En115 = 0x140149654});


    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode2Step<5, 12>(logger, entryPoint, paddingSpace, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    WriteInstruction32(codespace, 0x488d4db0); // lea rcx,[rbp-0x50]
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileExistsForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // if we have an injected file go to exit point, and write return value to RDX
    success.SetTarget(codespace);
    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RAX);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}

void InjectAtFileExists1(PatchExecData& execData, void* ffileExistsForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14006db8f, .En115 = 0x14006dc8f});
    char* const exitPointOnSuccess = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14006dba6, .En115 = 0x14006dca6});
    char* const paddingSpace = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14006cb52, .En115 = 0x14006cc52});


    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode2Step<5, 12>(logger, entryPoint, paddingSpace, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    WriteInstruction32(codespace, 0x488d4d80); // lea rcx,[rbp-0x80]
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileExistsForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // if we have an injected file go to exit point, and write return value to RDX
    success.SetTarget(codespace);
    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RAX);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}

void InjectAtFileExists2(PatchExecData& execData, void* ffileExistsForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14006de34, .En115 = 0x14006df34});
    char* const exitPointOnSuccess = GetCodeAddressEn(
        version,
        textRegion,
        Addresses{.En114 = 0x14006dba6, .En115 = 0x14006dca6}); // TODO: is this right?
    char* const paddingSpace = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x14006fb92, .En115 = 0x14006fc92});


    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode2Step<5, 12>(logger, entryPoint, paddingSpace, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    WriteInstruction32(codespace, 0x488d4d80); // lea rcx,[rbp-0x80]
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileExistsForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // if we have an injected file go to exit point, and write return value to RDX
    success.SetTarget(codespace);
    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RAX);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}

void InjectAtFFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140088b85, .En115 = 0x140089275});
    char* const exitPointOnSuccess = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140088bab, .En115 = 0x14008929b});
    char* const paddingSpace = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140088bf2, .En115 = 0x1400892e2});


    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode2Step<5, 12>(logger, entryPoint, paddingSpace, R64::RCX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_R64(codespace, R64::RDX, R64::R15);
    Emit_MOV_R64_R64(codespace, R64::RCX, R64::RBP);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    // check result
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // if we have an injected file go to exit point, and write return value to RBX
    success.SetTarget(codespace);
    Emit_MOV_R64_R64(codespace, R64::RBX, R64::RAX);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}

void InjectAtOpenFSoundFile(PatchExecData& execData, void* fsoundOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(
        version, textRegion, Addresses{.En114 = 0x140b7e1a0, .En115 = 0x140b74910});


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<15>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;


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

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen5
