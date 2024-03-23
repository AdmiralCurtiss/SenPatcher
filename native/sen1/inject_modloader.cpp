#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"

namespace SenLib::Sen1 {
void InjectAtFFileOpen(SenPatcher::Logger& logger,
                       char* textRegion,
                       GameVersion version,
                       char*& codespace,
                       char* codespaceEnd,
                       void* ffileOpenForwarder) {
    using namespace SenPatcher::x86;

    char* const entryPoint =
        textRegion
        + (version == GameVersion::Japanese ? (0x43ffe0 - 0x401000) : (0x440130 - 0x401000));

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_open_forwarder;
    ffile_open_forwarder.SetTarget(static_cast<char*>(ffileOpenForwarder));

    // call forwarder while preserving ecx
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::ESP, 4);
    Emit_PUSH_R32(codespace, R32::ECX);
    ffile_open_forwarder.WriteJump(codespace, JumpCondition::CALL);
    Emit_POP_R32(codespace, R32::ECX);

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
    Emit_RET_IMM16(codespace, 8);
}

void InjectAtFFileGetFilesize(SenPatcher::Logger& logger,
                              char* textRegion,
                              GameVersion version,
                              char*& codespace,
                              char* codespaceEnd,
                              void* ffileGetFilesizeForwarder) {
    using namespace SenPatcher::x86;

    char* const entryPoint =
        textRegion
        + (version == GameVersion::Japanese ? (0x440130 - 0x401000) : (0x440280 - 0x401000));

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_get_size_forwarder;
    ffile_get_size_forwarder.SetTarget(static_cast<char*>(ffileGetFilesizeForwarder));

    // call forwarder while preserving ecx
    Emit_PUSH_R32(codespace, R32::ECX);
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::ECX, R32::ESP, 8);
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::ESP, 16);
    ffile_get_size_forwarder.WriteJump(codespace, JumpCondition::CALL);
    Emit_POP_R32(codespace, R32::ECX);

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
}
} // namespace SenLib::Sen1
