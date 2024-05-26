#include "exe_patch.h"

#include <cassert>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchDlcCostumeCrash(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressEn(version, textRegion, 0x14022892d);
    char* const scratchAddress = GetCodeAddressEn(version, textRegion, 0x140228d64);
    char* const skipAddress = GetCodeAddressEn(version, textRegion, 0x140228a2d);

    char* tmp = execData.Codespace;
    const auto injectResult = InjectJumpIntoCode2Step<7, 12, PaddingInstruction::Nop>(
        logger, injectAddress, scratchAddress, R64::RDI, tmp);

    Emit_TEST_R64_R64(tmp, R64::RBX, R64::RBX);
    BranchHelper1Byte early_exit;
    early_exit.WriteJump(tmp, JumpCondition::JZ);

    std::memcpy(tmp,
                injectResult.OverwrittenInstructions.data(),
                injectResult.OverwrittenInstructions.size());
    tmp += injectResult.OverwrittenInstructions.size();
    Emit_MOV_R64_IMM64(tmp, R64::RDI, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(tmp, R64::RDI);

    early_exit.SetTarget(tmp);

    // write "???" as the generated string instead
    WriteInstruction24(tmp, 0xc74500); // mov dword ptr[rbp],0x3f3f3f
    HyoutaUtils::MemWrite::WriteAdvUInt32(tmp, 0x3f3f3f);
    Emit_MOV_R64_IMM64(tmp, R64::RDI, std::bit_cast<uint64_t>(skipAddress));
    Emit_JMP_R64(tmp, R64::RDI);

    execData.Codespace = tmp;
}
} // namespace SenLib::Sen5
