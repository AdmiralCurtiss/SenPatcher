#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"

#ifdef DEBUG_PR_FILE_LIFETIME
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

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
    Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::ECX, R32::ESP, 4);
    Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::EDX, R32::ESP, 12);
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
    Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::EDX, R32::EBP, 8);
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

    Emit_POP_R32(codespace, R32::ECX);
    WriteInstruction24(codespace, 0x894820); // mov dword ptr[eax+20h],ecx
    Emit_POP_R32(codespace, R32::ECX);
    WriteInstruction24(codespace, 0x894808); // mov dword ptr[eax+8h],ecx
    WriteInstruction24(codespace, 0x89481c); // mov dword ptr[eax+1ch],ecx

    BranchHelper4Byte success_exit;
    success_exit.SetTarget(exitPoint);
    success_exit.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}

void InjectAtPrFileExists(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    // note that there's a very similar function at 0x407bb0 that instead of returning true/false,
    // it returns a pointer to the struct. we intentionally do *not* inject there and only return
    // the vanilla BRA files for that -- that code is only reached in the 'no modded file, load
    // vanilla file' case, so it would be pointless to inject there.

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x408120, 0x407c60);

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_get_size_forwarder;
    ffile_get_size_forwarder.SetTarget(static_cast<char*>(ffileGetFilesizeForwarder));

    // call forwarder
    Emit_PUSH_R32(codespace, R32::ECX); // store this pointer
    Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::ECX, R32::ESP, 8);
    Emit_XOR_R32_R32(codespace, R32::EDX, R32::EDX);
    ffile_get_size_forwarder.WriteJump(codespace, JumpCondition::CALL);
    Emit_POP_R32(codespace, R32::ECX); // restore this pointer

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file return true
    success.SetTarget(codespace);
    Emit_MOV_R8_IMM8(codespace, R8::AL, 1);
    Emit_RET_IMM16(codespace, 4);

    execData.Codespace = codespace;
}

void InjectAtPrFileGetFilesize(PatchExecData& execData, void* ffileGetFilesizeForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x4080c0, 0x407c00);

    char* codespaceBegin = codespace;
    auto injectResult =
        InjectJumpIntoCode<7, PaddingInstruction::Nop>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    BranchHelper4Byte ffile_get_size_forwarder;
    ffile_get_size_forwarder.SetTarget(static_cast<char*>(ffileGetFilesizeForwarder));

    // call forwarder
    Emit_PUSH_R32(codespace, R32::ECX);    // store this pointer
    WriteInstruction16(codespace, 0x6a00); // push 0 ; create stack space for returned filesize
    Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::ECX, R32::ESP, 12);
    Emit_MOV_R32_R32(codespace, R32::EDX, R32::ESP);
    ffile_get_size_forwarder.WriteJump(codespace, JumpCondition::CALL);
    Emit_POP_R32(codespace, R32::EDX); // move returned filesize into edx
    Emit_POP_R32(codespace, R32::ECX); // restore this pointer

    // check result
    Emit_TEST_R32_R32(codespace, R32::EAX, R32::EAX);
    BranchHelper1Byte success;
    success.WriteJump(codespace, JumpCondition::JNS);

    // if we have no injected file proceed with normal function
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // if we have an injected file just return immediately
    success.SetTarget(codespace);
    Emit_MOV_R32_R32(codespace, R32::EAX, R32::EDX);
    Emit_RET_IMM16(codespace, 4);

    execData.Codespace = codespace;
}

#ifdef DEBUG_PR_FILE_LIFETIME
namespace {
struct PrFileHandle {
    void* vfuncptr;
    uint32_t Unknown1;
    uint32_t DataLength;
};
static_assert(offsetof(PrFileHandle, vfuncptr) == 0);
static_assert(offsetof(PrFileHandle, DataLength) == 0x8);
} // namespace

static void __fastcall LogConstructedPrFile(PrFileHandle* handle, const char* filename) {
    char buffer[512];

    if (!handle) {
        sprintf(buffer, "Returning null PrFile for: %s\n", filename);
    } else {
        sprintf(buffer,
                "Constructed PrFile at 0x%08x (vtbl 0x%08x, filesize %u): %s\n",
                std::bit_cast<uint32_t>(handle),
                std::bit_cast<uint32_t>(handle->vfuncptr),
                handle->DataLength,
                filename);
    }

    OutputDebugStringA(buffer);
}

static void __fastcall LogDestructedPrFile(PrFileHandle* handle) {
    char buffer[512];

    sprintf(buffer,
            "Destructing PrFile at 0x%08x (vtbl 0x%08x, filesize %u)\n",
            std::bit_cast<uint32_t>(handle),
            std::bit_cast<uint32_t>(handle->vfuncptr),
            handle->DataLength);

    OutputDebugStringA(buffer);
}

void InjectDebugCodeForPrFileLifetime(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    char* const openPrFileReturn = GetCodeAddressSteamGog(version, textRegion, 0, 0x408403);
    char* const prFileHandleDtor = GetCodeAddressSteamGog(version, textRegion, 0, 0x40f9c3);
    char* const prFileHandleMemDtor = GetCodeAddressSteamGog(version, textRegion, 0, 0x4100e6);
    char* const prFileHandleZipDtor = GetCodeAddressSteamGog(version, textRegion, 0, 0x411560);

    // {
    //     // leak audio files
    //     char* addr = GetCodeAddressSteamGog(version, textRegion, 0, 0x446ea2);
    //     PageUnprotect page(logger, addr, 1);
    //     *addr = 0xeb;
    // }

    {
        auto injectResult = InjectJumpIntoCode<6>(logger, openPrFileReturn, codespace);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        BranchHelper4Byte log_func;
        void* logOpenedPrFileFunc = LogConstructedPrFile;
        log_func.SetTarget(static_cast<char*>(logOpenedPrFileFunc));

        Emit_PUSH_R32(codespace, R32::EAX);
        Emit_MOV_R32_DwordPtrR32PlusOffset(codespace, R32::EDX, R32::EBP, 8);
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::EAX);
        log_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::EAX);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
    }

    {
        auto injectResult = InjectJumpIntoCode<5>(logger, prFileHandleDtor, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        BranchHelper4Byte log_func;
        void* logDestructedPrFileFunc = LogDestructedPrFile;
        log_func.SetTarget(static_cast<char*>(logDestructedPrFileFunc));

        Emit_PUSH_R32(codespace, R32::ECX);
        log_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::ECX);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    {
        auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, prFileHandleMemDtor, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        BranchHelper4Byte log_func;
        void* logDestructedPrFileFunc = LogDestructedPrFile;
        log_func.SetTarget(static_cast<char*>(logDestructedPrFileFunc));

        Emit_PUSH_R32(codespace, R32::ECX);
        log_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::ECX);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    {
        auto injectResult =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, prFileHandleZipDtor, codespace);
        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        BranchHelper4Byte log_func;
        void* logDestructedPrFileFunc = LogDestructedPrFile;
        log_func.SetTarget(static_cast<char*>(logDestructedPrFileFunc));

        Emit_PUSH_R32(codespace, R32::ECX);
        log_func.WriteJump(codespace, JumpCondition::CALL);
        Emit_POP_R32(codespace, R32::ECX);

        jump_back.WriteJump(codespace, JumpCondition::JMP);
    }

    execData.Codespace = codespace;
}
#endif
} // namespace SenLib::TX
