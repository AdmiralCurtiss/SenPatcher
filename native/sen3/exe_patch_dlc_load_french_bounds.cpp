#include "exe_patch.h"

#include <cassert>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace SenLib::Sen3 {
static void DebugPrintFree(uint64_t address) {
    char buffer[256];
    sprintf(buffer, "Thread %d: free(0x%016llx)\n", GetCurrentThreadId(), address);
    OutputDebugStringA(buffer);
}

void PatchDlcLoadFrenchBounds(PatchExecData& execData) {
    if (execData.Version == GameVersion::Japanese) {
        // Japanese version doesn't have this bug
        return;
    }

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressJpEn(version, textRegion, 0, 0x1403a99c2);
    static constexpr size_t injectLength = 0x17;

    char* tmp = execData.Codespace;
    const auto injectResult = InjectJumpIntoCode<injectLength, PaddingInstruction::Nop>(
        logger, injectAddress, R64::RCX, tmp);

    // the original code here effectively does:
    // for (uint32_t i = 0; i < dlc_table_english->number_of_entries; ++i) {
    //     dlc_table_english.entries[i].some_field |= 1;
    //     dlc_table_french.entries[i].some_field |= 1;
    // }
    // without ever checking that the dlc_table_french has the same length as the english one.
    // so we'll just split this up into two loops instead.

    BranchHelper1Byte english_loop;
    english_loop.SetTarget(tmp);
    WriteInstruction24(tmp, 0x830a01);   // or dword ptr[rdx],1
    WriteInstruction32(tmp, 0x488d5270); // lea rdx,[rdx + 0x70]
    WriteInstruction24(tmp, 0x41ffc0);   // inc r8d
    WriteInstruction32(tmp, 0x453b4738); // cmp r8d,dword ptr[r15 + 0x38]
    english_loop.WriteJump(tmp, JumpCondition::JC);

    Emit_XOR_R64_R64(tmp, R64::R8, R64::R8);

    BranchHelper1Byte french_loop;
    french_loop.SetTarget(tmp);
    WriteInstruction24(tmp, 0x830801);   // or dword ptr[rax],1
    WriteInstruction32(tmp, 0x488d4070); // lea rax,[rax + 0x70]
    WriteInstruction24(tmp, 0x41ffc0);   // inc r8d
    WriteInstruction32(tmp, 0x443b4638); // cmp r8d,dword ptr[rsi + 0x38]
    french_loop.WriteJump(tmp, JumpCondition::JC);

    Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(tmp, R64::RCX);

    execData.Codespace = tmp;
}
} // namespace SenLib::Sen3
