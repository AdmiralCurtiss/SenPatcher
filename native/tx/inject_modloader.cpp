#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"

// namespace {
// struct PrFileHandleMem {
//     void* vfuncptr;
//     uint32_t Unknown1;
//     uint32_t DataLength;
//     uint32_t Unknown3;
//     uint32_t Unknown4;
//     uint32_t Unknown5;
//     uint32_t Position;
//     uint32_t BufferSize;
//     void* DataBuffer;
//     uint8_t ShouldFreeBufferOnDestruction;
//     uint8_t Padding1;
//     uint8_t Padding2;
//     uint8_t Padding3;
//     void* PointerToEntryInCache; // something like this...
//     uint32_t Unknown6;
// };
// static_assert(offsetof(PrFileHandleMem, vfuncptr) == 0);
// static_assert(offsetof(PrFileHandleMem, DataLength) == 0x8);
// static_assert(offsetof(PrFileHandleMem, Position) == 0x18);
// static_assert(offsetof(PrFileHandleMem, BufferSize) == 0x1c);
// static_assert(offsetof(PrFileHandleMem, DataBuffer) == 0x20);
// static_assert(offsetof(PrFileHandleMem, ShouldFreeBufferOnDestruction) == 0x24);
// static_assert(offsetof(PrFileHandleMem, PointerToEntryInCache) == 0x28);
// } // namespace

namespace SenLib::TX {
void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x44b94c, 0x44a29c);
    char* const exitPoint = GetCodeAddressSteamGog(version, textRegion, 0x44b95b, 0x44a2ab);

    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, entryPoint, codespaceBegin);
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

void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x43e64a, 0x43ce7a);
    char* const compressionFlagCheck1 =
        GetCodeAddressSteamGog(version, textRegion, 0x43e548, 0x43cd78);
    char* const compressionFlagCheck2 =
        GetCodeAddressSteamGog(version, textRegion, 0x4272e7, 0x425cd7);

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

void InjectAtPrFileOpen(PatchExecData& execData, void* prFileOpenForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x4082e8, 0x407e28);
    // char* const exitPoint = GetCodeAddressSteamGog(version, textRegion, 0x4088a3, 0x4083e3);
    char* const exitPoint = GetCodeAddressSteamGog(version, textRegion, 0x40836e, 0x407eae);
    char* const prFileHandleMemCtor =
        GetCodeAddressSteamGog(version, textRegion, 0x410cd0, 0x40fd90);
    char* const prFileHandleMemAllocCode = entryPoint + 0x3f;
    const size_t prFileHandleMemAllocLength = 2 + 6 + 3;

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte prfile_open_forwarder;
    prfile_open_forwarder.SetTarget(static_cast<char*>(prFileOpenForwarder));

    // make some stack space for the PrFileHelperStruct, initialize to zero
    Emit_XOR_R32_R32(codespace, R32::EAX, R32::EAX);
    Emit_PUSH_R32(codespace, R32::EAX);
    Emit_PUSH_R32(codespace, R32::EAX);

    // call forwarder
    Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::EBP, 8);
    Emit_MOV_R32_R32(codespace, R32::ECX, R32::ESP);
    prfile_open_forwarder.WriteJump(codespace, JumpCondition::CALL);

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    Emit_ADD_R32_IMM32(codespace, R32::ESP, 8);
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file allocate a prFileHandleMem and then go to exit point.
    // the prFileHandleMem pointer goes into EAX, the rest will handle itself.
    // this crashes if the allocation fails, but I think we're dead if that happens anyway...
    success.SetTarget(codespace);
    std::memcpy(codespace, prFileHandleMemAllocCode, prFileHandleMemAllocLength);
    codespace += prFileHandleMemAllocLength;

    Emit_MOV_R32_R32(codespace, R32::ECX, R32::EAX);
    BranchHelper4Byte init_prFileHandleMem;
    init_prFileHandleMem.SetTarget(prFileHandleMemCtor);
    init_prFileHandleMem.WriteJump(codespace, JumpCondition::CALL);

    WriteInstruction32(codespace, 0x8b4c2404); // mov ecx,dword ptr[esp+4]
    WriteInstruction24(codespace, 0x894808);   // mov dword ptr[eax+8h],ecx
    WriteInstruction24(codespace, 0x89481c);   // mov dword ptr[eax+1ch],ecx
    WriteInstruction24(codespace, 0x8b0c24);   // mov ecx,dword ptr[esp]
    WriteInstruction24(codespace, 0x894820);   // mov dword ptr[eax+20h],ecx

    Emit_ADD_R32_IMM32(codespace, R32::ESP, 8);
    BranchHelper4Byte success_exit;
    success_exit.SetTarget(exitPoint);
    success_exit.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
