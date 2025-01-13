#include "exe_patch.h"

#include <cassert>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen4 {
void PatchDlcCostumeCrash(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressJpEn(version,
                                                   textRegion,
                                                   Addresses{.Jp121 = 0x140203d0d,
                                                             .En121 = 0x140205f80,
                                                             .Jp122 = 0x140203c6d,
                                                             .En122 = 0x140205ee0});
    char* const scratchAddress = GetCodeAddressJpEn(version,
                                                    textRegion,
                                                    Addresses{.Jp121 = 0x140204044,
                                                              .En121 = 0x140206052,
                                                              .Jp122 = 0x140203fa4,
                                                              .En122 = 0x140205fb2});
    char* const skipAddress = GetCodeAddressJpEn(version,
                                                 textRegion,
                                                 Addresses{.Jp121 = 0x140203d5b,
                                                           .En121 = 0x140205fce,
                                                           .Jp122 = 0x140203cbb,
                                                           .En122 = 0x140205f2e});

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
    WriteInstruction32(tmp, 0x488d8c24); // lea rcx,[rsp+0xb0]
    HyoutaUtils::MemWrite::WriteAdvUInt32(tmp, 0xb0);
    WriteInstruction16(tmp, 0xc701); // mov dword ptr[rcx],0x3f3f3f
    HyoutaUtils::MemWrite::WriteAdvUInt32(tmp, 0x3f3f3f);
    Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(skipAddress));
    Emit_JMP_R64(tmp, R64::RCX);

    execData.Codespace = tmp;
}
} // namespace SenLib::Sen4
