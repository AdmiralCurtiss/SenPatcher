#include "inject_modloader.h"

#include <bit>
#include <cstdint>
#include <cstring>

#include "exe_patch.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"

// NOTE: When calling a function, stack must be aligned to 0x10 bytes!

// #define HOOK_MAGIC_DESCRIPTION_GENERATOR
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace SenLib::Sen3 {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
static void HookMagicDescriptionGenerator(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPointSystemMenu =
        GetCodeAddressJpEn(version, textRegion, Addresses{.En106 = 0x14033ef1e});
    char* const entryPointDivineKnightBattle =
        GetCodeAddressJpEn(version, textRegion, Addresses{.En106 = 0x14013f5c6});
    char* const freeSpaceDivineKnightBattle =
        GetCodeAddressJpEn(version, textRegion, Addresses{.En106 = 0x14013f912});

    void* handle = CreateFileMappingW(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 0x1000, L"SenMagicDescGenHook");
    if (handle == nullptr) {
        return;
    }

    void* memory = MapViewOfFileEx(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000, nullptr);
    if (memory == nullptr) {
        return;
    }

    {
        auto injectResult =
            InjectJumpIntoCode<12>(logger, entryPointSystemMenu, R64::RAX, codespace);
        char* inject = injectResult.JumpBackAddress;
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(memory));
        WriteInstruction16(codespace, 0x8b18); // mov ebx,dword ptr[rax]
        Emit_TEST_R64_R64(codespace, R64::RBX, R64::RBX);
        BranchHelper1Byte branch;
        branch.WriteJump(codespace, JumpCondition::JZ);

        WriteInstruction24(codespace, 0x8b5004); // mov edx,dword ptr[rax+4]
        Emit_MOV_R64_R64(codespace, R64::RDI, R64::RDX);

        branch.SetTarget(codespace);

        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();
        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
        Emit_JMP_R64(codespace, R64::RAX);
    }
    {
        auto injectResult = InjectJumpIntoCode2Step<7, 12>(
            logger, entryPointDivineKnightBattle, freeSpaceDivineKnightBattle, R64::RDX, codespace);
        char* inject = injectResult.JumpBackAddress;
        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(memory));
        WriteInstruction16(codespace, 0x8b10); // mov edx,dword ptr[rax]
        Emit_TEST_R64_R64(codespace, R64::RDX, R64::RDX);
        BranchHelper1Byte keep;
        keep.WriteJump(codespace, JumpCondition::JZ);

        WriteInstruction24(codespace, 0x8b5004); // mov edx,dword ptr[rax+4]

        BranchHelper1Byte overwrite;
        overwrite.WriteJump(codespace, JumpCondition::JMP);

        keep.SetTarget(codespace);

        std::memcpy(codespace, overwrittenInstructions.data() + 4, 3);
        codespace += 3;

        overwrite.SetTarget(codespace);

        std::memcpy(codespace, overwrittenInstructions.data(), 4);
        codespace += 4;

        Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
        Emit_JMP_R64(codespace, R64::RAX);
    }

    execData.Codespace = codespace;
}
#endif

void InjectAtFFileOpen(PatchExecData& execData, void* ffileOpenForwarder) {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
    HookMagicDescriptionGenerator(execData);
#endif

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version,
                                                textRegion,
                                                Addresses{.Jp106 = 0x1401316f3,
                                                          .En106 = 0x140134e83,
                                                          .Jp107 = 0x140131673,
                                                          .En107 = 0x140134e03});
    char* const exitPoint = GetCodeAddressJpEn(version,
                                               textRegion,
                                               Addresses{.Jp106 = 0x140131756,
                                                         .En106 = 0x140134ee6,
                                                         .Jp107 = 0x1401316d6,
                                                         .En107 = 0x140134e66});


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
                                                Addresses{.Jp106 = 0x140131630,
                                                          .En106 = 0x140134dc0,
                                                          .Jp107 = 0x1401315b0,
                                                          .En107 = 0x140134d40});
    char* const exitPoint = GetCodeAddressJpEn(version,
                                               textRegion,
                                               Addresses{.Jp106 = 0x14013166c,
                                                         .En106 = 0x140134dfc,
                                                         .Jp107 = 0x1401315ec,
                                                         .En107 = 0x140134d7c});


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
                                                Addresses{.Jp106 = 0x140086900,
                                                          .En106 = 0x140086900,
                                                          .Jp107 = 0x140086900,
                                                          .En107 = 0x140086900});


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

void InjectAtDecompressPkg(PatchExecData& execData, void* decompressPkgForwarder) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;

    char* const entryPoint = GetCodeAddressJpEn(version,
                                                textRegion,
                                                Addresses{.Jp106 = 0x140104158,
                                                          .En106 = 0x1401047de,
                                                          .Jp107 = 0x140104158,
                                                          .En107 = 0x1401047de});
    char* const compressionFlagCheckForDecompression =
        GetCodeAddressJpEn(version,
                           textRegion,
                           Addresses{.Jp106 = 0x1401040ab,
                                     .En106 = 0x140104731,
                                     .Jp107 = 0x1401040ab,
                                     .En107 = 0x140104731});
    char* const compressionFlagCheckForSomethingElse =
        GetCodeAddressJpEn(version,
                           textRegion,
                           Addresses{.Jp106 = 0x1400e0deb,
                                     .En106 = 0x1400e0e9b,
                                     .Jp107 = 0x1400e0deb,
                                     .En107 = 0x1400e0e9b});

    // TODO: Is that all the checks for pkg flags?

    {
        // this changes a '(flags & 1) != 0' check to a '(flags & 0x81) != 0' check.
        // this causes the code flow to proceed towards the type1 extraction (which we inject at)
        // for both that and the fake pka ref 'compression'
        char* tmp = compressionFlagCheckForDecompression;
        PageUnprotect page(logger, tmp, 2);
        *(tmp + 1) = (char)0x81;
    }
    {
        // same as above, except this is checking for any compression, not specifically for type1,
        // so we update the check for all compression flags from 0x1d to 0x9d.
        // (bit 0 -> type1, bit 2 -> lz4, bit 3 -> lzma, bit 4 -> zstd, bit 7 -> pka ref)
        char* tmp = compressionFlagCheckForSomethingElse;
        PageUnprotect page(logger, tmp, 4);
        *(tmp + 3) = (char)0x9d;
    }

    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<12, PaddingInstruction::Nop>(
        logger, entryPoint, R64::RDX, codespaceBegin);

    // we have overwritten:
    // - test rax,rax
    // - jz output_buffer_allocation_failure
    // - mov rdx,rax
    // reconstruct this code
    std::memcpy(codespace, injectResult.OverwrittenInstructions.data(), 3);
    codespace += 3;
    BranchHelper1Byte output_buffer_allocation_success;
    output_buffer_allocation_success.WriteJump(codespace, JumpCondition::JNZ);
    int32_t relativeTarget;
    std::memcpy(&relativeTarget, injectResult.OverwrittenInstructions.data() + 5, 4);
    char* failureJumpBackAddress = (injectResult.JumpBackAddress - 3) + relativeTarget;
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(failureJumpBackAddress));
    Emit_JMP_R64(codespace, R64::RDX);

    output_buffer_allocation_success.SetTarget(codespace);

    // r12/rcx is the input bufer
    // rdx/rax is the output buffer
    // r9/rdi is the pointer to the single file pkg header

    // allocation was successful, so we can call our replacement decompressor
    // rcx is already set
    Emit_MOV_R64_R64(codespace, R64::RDX, R64::RAX);
    Emit_MOV_R64_R64(codespace, R64::R8, R64::RDI);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(decompressPkgForwarder));
    Emit_CALL_R64(codespace, R64::RAX);

    // return value in eax is already correct
    // skip the original decompression call and go back
    char* successJumpBackAddress = (injectResult.JumpBackAddress + 5);
    Emit_MOV_R64_IMM64(codespace, R64::RCX, std::bit_cast<uint64_t>(successJumpBackAddress));
    Emit_JMP_R64(codespace, R64::RCX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen3
