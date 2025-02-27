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
    char* const exitPoint = GetCodeAddressJpEn(version, textRegion, 0x482630, 0x483ea0);

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
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file go to return code, return value is already correct in EAX
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

void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressJpEn(version, textRegion, 0x44feee, 0x45003e);
    char* const compressionFlagCheck1 = GetCodeAddressJpEn(version, textRegion, 0x44fe49, 0x44ff99);
    char* const compressionFlagCheck2 = GetCodeAddressJpEn(version, textRegion, 0x425078, 0x4250f8);
    char* const compressionFlagCheck3 = GetCodeAddressJpEn(version, textRegion, 0x425301, 0x425381);

    // TODO: Is that all the '(flags & 1) != 0' checks?

    {
        // this changes a '(flags & 1) != 0' check to a '(flags & 0x9d) != 0' check
        // that way compression is detected if any non-checksum compression flag is set, and not
        // just bit 0 (bit 0 -> type1, bit 2 -> lz4, bit 3 -> lzma, bit 4 -> zstd, bit 7 -> pka ref)
        char* tmp = compressionFlagCheck1;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0x9d;
    }
    {
        char* tmp = compressionFlagCheck2;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0x9d;
    }
    {
        char* tmp = compressionFlagCheck3;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0x9d;
    }

    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode<7, PaddingInstruction::Nop>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);

    BranchHelper4Byte decompress_pkg_forwarder;
    decompress_pkg_forwarder.SetTarget(static_cast<char*>(decompressPkgForwarder));

    // call forwarder
    Emit_MOV_R32_R32(codespace, R32::ECX, R32::EBX);
    Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
    Emit_PUSH_R32(codespace, R32::EDI);
    decompress_pkg_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // fix stack
    Emit_SUB_R32_IMM32(codespace, R32::ESP, 0x8);

    // no return value to check, so just go back
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}

} // namespace SenLib::Sen1
