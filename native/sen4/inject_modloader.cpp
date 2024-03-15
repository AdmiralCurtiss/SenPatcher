#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

namespace SenLib::Sen4 {
void InjectAtFFileOpen(SenPatcher::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                 : (0x1400af7a0 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                : (0x1400af8bd - 0x140001000));


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespaceBegin);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    Emit_PUSH_R64(codespace, R64::RAX);
    Emit_PUSH_R64(codespace, R64::RCX);
    Emit_PUSH_R64(codespace, R64::RDX);
    Emit_PUSH_R64(codespace, R64::R8);

    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);
    Emit_POP_R64(codespace, R64::RAX);

    // check for success
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::RDI, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RDI);

    // return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtFFileGetFilesize(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                 : (0x1400af656 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                : (0x1400af756 - 0x140001000));


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<13>(logger, entryPoint, R64::RAX, codespaceBegin);
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

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(ffileGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on fail, go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // on success, return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtFreestandingGetFilesize(SenPatcher::Logger& logger,
                                     char* textRegion,
                                     GameVersion version,
                                     char*& codespace,
                                     char* codespaceEnd,
                                     void* freestandingGetFilesizeForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                 : (0x1400aed85 - 0x140001000));
    char* const exitPoint = textRegion
                            + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                : (0x1400aeeae - 0x140001000));


    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespaceBegin);
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

    Emit_MOV_R64_IMM64(
        codespace, R64::RAX, std::bit_cast<uint64_t>(freestandingGetFilesizeForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // go back to function and pretend nothing happened
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // return that we succeeded
    success.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, 1);

    // jump to exit point
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RCX);
}

void InjectAtOpenFSoundFile(SenPatcher::Logger& logger,
                            char* textRegion,
                            GameVersion version,
                            char*& codespace,
                            char* codespaceEnd,
                            void* fsoundOpenForwarder) {
    using namespace SenPatcher::x64;

    char* const entryPoint = textRegion
                             + (version == GameVersion::Japanese ? (00000000000 - 0x140001000)
                                                                 : (0x14080f430 - 0x140001000));


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
    Emit_SUB_R64_IMM32(codespace, R64::RSP, 0x20);

    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(fsoundOpenForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    Emit_ADD_R64_IMM32(codespace, R64::RSP, 0x20);
    Emit_POP_R64(codespace, R64::R11);
    Emit_POP_R64(codespace, R64::R10);
    Emit_POP_R64(codespace, R64::R9);
    Emit_POP_R64(codespace, R64::R8);
    Emit_POP_R64(codespace, R64::RDX);
    Emit_POP_R64(codespace, R64::RCX);

    // check for success
    Emit_TEST_R64_R64(codespace, R64::RAX, R64::RAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNZ);

    // on failure go back to function
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // on success just return from the function, RAX already has the correct return value
    success.SetTarget(codespace);
    Emit_RET(codespace);
}
} // namespace SenLib::Sen4
