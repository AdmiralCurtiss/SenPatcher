#include "exe_patch.h"

#include <cassert>

#include "util/memread.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen5 {
void PatchShowMouseCursor(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x64;
    char* showCursorPos1 = GetCodeAddressEn(version, textRegion, 0x140838c46);
    char* showCursorPos1Scratch = GetCodeAddressEn(version, textRegion, 0x1408389e2);
    // char* showCursorPos2 = GetCodeAddressEn(version, textRegion, 0x14088135b); // never called?
    // char* showCursorPos3 = GetCodeAddressEn(version, textRegion, 0x140881339); // never called?

    // only allow call to SetCursor() if the parameter is not null
    {
        char* tmp = codespace;
        auto inject = InjectJumpIntoCode2Step<6, 12, PaddingInstruction::Nop>(
            logger, showCursorPos1, showCursorPos1Scratch, R64::RAX, tmp);
        const int32_t relativeOffsetForCall = static_cast<int32_t>(
            HyoutaUtils::MemRead::ReadUInt32(&inject.OverwrittenInstructions[2]));
        const char* absolutePositionForCall = inject.JumpBackAddress + relativeOffsetForCall;

        // check rcx, if != 0 call then jump back, else just jump back
        Emit_TEST_R64_R64(tmp, R64::RCX, R64::RCX);
        BranchHelper1Byte skip;
        skip.WriteJump(tmp, JumpCondition::JZ);
        Emit_MOV_R64_IMM64(tmp, R64::RAX, std::bit_cast<uint64_t>(absolutePositionForCall));
        WriteInstruction24(tmp, 0x488b00u); // mov rax,qword ptr[rax]
        Emit_CALL_R64(tmp, R64::RAX);
        skip.SetTarget(tmp);
        Emit_MOV_R64_IMM64(tmp, R64::RCX, std::bit_cast<uint64_t>(inject.JumpBackAddress));
        Emit_JMP_R64(tmp, R64::RCX);

        codespace = tmp;
    }

    // // remove call to SetCursor(nullptr)
    // {
    //     char* tmp = showCursorPos3;
    //     PageUnprotect page(logger, tmp, 6);
    //     for (size_t i = 0; i < 6; ++i) {
    //         *tmp++ = static_cast<char>(0x90); // nop
    //     }
    // }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen5
