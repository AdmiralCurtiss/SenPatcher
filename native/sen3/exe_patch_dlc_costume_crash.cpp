#include "exe_patch.h"

#include <cassert>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDlcCostumeCrash(HyoutaUtils::Logger& logger,
                          char* textRegion,
                          GameVersion version,
                          char*& codespace,
                          char* codespaceEnd) {
    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressJpEn(version, textRegion, 0x14022ae3b, 0x140231166);
    char* const scratchAddress = GetCodeAddressJpEn(version, textRegion, 0x14022bd64, 0x1402320e4);
    char* const skipAddress = GetCodeAddressJpEn(version, textRegion, 0x14022ae8c, 0x1402311b7);

    char*& tmp = codespace;
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
}
} // namespace SenLib::Sen3
