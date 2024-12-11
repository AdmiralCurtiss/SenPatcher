#include "exe_patch.h"

#include <cassert>
#include <cstring>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace SenLib::Sen3 {
void PatchDlcLoadFrenchBounds(PatchExecData& execData) {
    if (execData.Version == GameVersion::Japanese) {
        // Japanese version doesn't have this bug
        return;
    }

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressJpEn(version, textRegion, 0, 0x1403a9942);
    static constexpr size_t injectLength = 0x17;
    char* const frenchNonzeroCheckAddress = GetCodeAddressJpEn(version, textRegion, 0, 0x1403a9921);
    static constexpr size_t frenchNonzeroCheckLength = 10;

    // the original code here effectively does:
    // if (dlc_table_english->number_of_entries != 0 && dlc_table_french->number_of_entries != 0) {
    //     for (uint32_t i = 0; i < dlc_table_english->number_of_entries; ++i) {
    //         dlc_table_english->entries[i].some_field |= 1;
    //         dlc_table_french->entries[i].some_field |= 1;
    //     }
    //     // process dlc items
    // }
    // without ever checking that the dlc_table_french has the same length as the english one.
    // so we'll just split this up into two loops instead.

    // remove the dlc_table_french->number_of_entries != 0 test so that DLC loads fine even if you
    // have no french DLC files
    {
        PageUnprotect page(logger, frenchNonzeroCheckAddress, frenchNonzeroCheckLength);
        std::memset(frenchNonzeroCheckAddress, 0x90, frenchNonzeroCheckLength);
    }

    // split the loop into two
    char* tmp = execData.Codespace;
    const auto injectResult = InjectJumpIntoCode<injectLength, PaddingInstruction::Nop>(
        logger, injectAddress, R64::RCX, tmp);

    // this code is never reached if number of entries is zero, so no need to check for that
    BranchHelper1Byte english_loop;
    english_loop.SetTarget(tmp);
    WriteInstruction24(tmp, 0x830a01);   // or dword ptr[rdx],1
    WriteInstruction32(tmp, 0x488d5270); // lea rdx,[rdx + 0x70]
    WriteInstruction24(tmp, 0x41ffc0);   // inc r8d
    WriteInstruction32(tmp, 0x453b4738); // cmp r8d,dword ptr[r15 + 0x38]
    english_loop.WriteJump(tmp, JumpCondition::JC);

    Emit_XOR_R64_R64(tmp, R64::R8, R64::R8);

    // this, however, needs to be checked for zero
    BranchHelper1Byte french_loop;
    BranchHelper1Byte french_skip;
    WriteInstruction32(tmp, 0x443b4638); // cmp r8d,dword ptr[rsi + 0x38]
    french_skip.WriteJump(tmp, JumpCondition::JE);
    french_loop.SetTarget(tmp);
    WriteInstruction24(tmp, 0x830801);   // or dword ptr[rax],1
    WriteInstruction32(tmp, 0x488d4070); // lea rax,[rax + 0x70]
    WriteInstruction24(tmp, 0x41ffc0);   // inc r8d
    WriteInstruction32(tmp, 0x443b4638); // cmp r8d,dword ptr[rsi + 0x38]
    french_loop.WriteJump(tmp, JumpCondition::JC);
    french_skip.SetTarget(tmp);

    Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(tmp, R64::RCX);

    execData.Codespace = tmp;
}
} // namespace SenLib::Sen3
