#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

namespace SenLib::Sen5 {
void InjectAtFFileOpen(HyoutaUtils::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x140088732);
    char* const exitPoint = GetCodeAddressEn(version, textRegion, 0x14008877a);


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

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // jump to exit point (success is set by code at exit point)
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RDX);
}

void InjectAtBattleScriptExists(HyoutaUtils::Logger& logger,
                                char* textRegion,
                                GameVersion version,
                                char*& codespace,
                                char* codespaceEnd,
                                void* ffileExistsForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x14014844a);
    char* const exitPointOnSuccess = GetCodeAddressEn(version, textRegion, 0x140148412);
    char* const paddingSpace = GetCodeAddressEn(version, textRegion, 0x140149de4);


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

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // jump to exit point (success is set by code at exit point)
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtFileExists1(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x14006db8f);
    char* const exitPointOnSuccess = GetCodeAddressEn(version, textRegion, 0x14006db63);
    char* const paddingSpace = GetCodeAddressEn(version, textRegion, 0x14006cb52);


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

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // jump to exit point (success is set by code at exit point)
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtFileExists2(HyoutaUtils::Logger& logger,
                         char* textRegion,
                         GameVersion version,
                         char*& codespace,
                         char* codespaceEnd,
                         void* ffileExistsForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x14006de34);
    char* const exitPointOnSuccess = GetCodeAddressEn(version, textRegion, 0x14006de08);
    char* const paddingSpace = GetCodeAddressEn(version, textRegion, 0x14006fb92);


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

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // jump to exit point (success is set by code at exit point)
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtFFileGetFilesize(HyoutaUtils::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x140088b85);
    char* const exitPointOnSuccess = GetCodeAddressEn(version, textRegion, 0x140088ba9);
    char* const paddingSpace = GetCodeAddressEn(version, textRegion, 0x140088bf2);


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

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RCX);

    // jump to exit point (success is set by code at exit point)
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPointOnSuccess));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtOpenFSoundFile(HyoutaUtils::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd,
                            void* fsoundOpenForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressEn(version, textRegion, 0x140b7e1a0);


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<15>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(fsoundOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure go back to function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // on success just return from the function, RAX already has the correct return value
    success.SetTarget(codespace);
    Emit_RET(codespace);
}
} // namespace SenLib::Sen5
