#include "exe_patch.h"

#include <cassert>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDlcCostumeCrash(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    // TODO? There's a second block of code that's practically identical, should we patch this too?
    char* const injectAddress = GetCodeAddressJpEn(version,
                                                   textRegion,
                                                   Addresses{.Jp106 = 0x14022ae3b,
                                                             .En106 = 0x140231166,
                                                             .Jp107 = 0x14022adbb,
                                                             .En107 = 0x1402310e6});
    char* const scratchAddress = GetCodeAddressJpEn(version,
                                                    textRegion,
                                                    Addresses{.Jp106 = 0x14022bd64,
                                                              .En106 = 0x1402320e4,
                                                              .Jp107 = 0x14022bce4,
                                                              .En107 = 0x140232064});
    char* const skipAddress = GetCodeAddressJpEn(version,
                                                 textRegion,
                                                 Addresses{.Jp106 = 0x14022ae8c,
                                                           .En106 = 0x1402311b7,
                                                           .Jp107 = 0x14022ae0c,
                                                           .En107 = 0x140231137});

    char* tmp = execData.Codespace;
    const auto injectResult = InjectJumpIntoCode2Step<7, 12, PaddingInstruction::Nop>(
        logger, injectAddress, scratchAddress, R64::RCX, tmp);

    Emit_TEST_R64_R64(tmp, R64::RAX, R64::RAX);
    BranchHelper1Byte early_exit;
    early_exit.WriteJump(tmp, JumpCondition::JZ);

    std::memcpy(tmp,
                injectResult.OverwrittenInstructions.data(),
                injectResult.OverwrittenInstructions.size());
    tmp += injectResult.OverwrittenInstructions.size();
    Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(tmp, R64::RCX);

    early_exit.SetTarget(tmp);

    // write "???" as the generated string instead
    WriteInstruction32(tmp, 0x488d8c24); // lea rcx,[rsp+0x4b0]
    HyoutaUtils::MemWrite::WriteAdvUInt32(tmp, 0x4b0);
    WriteInstruction16(tmp, 0xc701); // mov, dword ptr[rcx],0x3f3f3f
    HyoutaUtils::MemWrite::WriteAdvUInt32(tmp, 0x3f3f3f);
    Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(skipAddress));
    Emit_JMP_R64(tmp, R64::RCX);

    execData.Codespace = tmp;
}

void PatchDlcSaveFileLoad(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* injectPos = GetCodeAddressJpEn(version,
                                         textRegion,
                                         Addresses{.Jp106 = 0x1403bba05,
                                                   .En106 = 0x1403c5c75,
                                                   .Jp107 = 0x1403bb985,
                                                   .En107 = 0x1403c5bf5});

    {
        char* tmp = injectPos;
        PageUnprotect page(logger, tmp, 1);
        *tmp = static_cast<char>(0xeb); // jnc -> jmp
    }
}
} // namespace SenLib::Sen3
