#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
void PatchValidDlcIds(PatchExecData& execData, char* validDlcBitfield, size_t maxDlcId) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x86;
    char* incCmpJmpHookAddress = GetCodeAddressSteamGog(version, textRegion, 0x5f37b2, 0x5f1c72);
    char* pushIdHookAddress = GetCodeAddressSteamGog(version, textRegion, 0x5f376c, 0x5f1c2c);

    {
        char* codespace = execData.Codespace;

        // increment counter by 1 and compare to maxDlcId
        auto inject1 =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, incCmpJmpHookAddress, codespace);

        BranchHelper1Byte skipDlcCheck;
        skipDlcCheck.SetTarget(codespace);
        WriteInstruction8(codespace, 0x46);    // inc esi
        WriteInstruction16(codespace, 0x81fe); // cmp esi,maxDlcId
        uint32_t maxDlcIdU32 = maxDlcId;
        std::memcpy(codespace, &maxDlcIdU32, 4);
        codespace += 4;

        BranchHelper4Byte jumpBack1;
        jumpBack1.SetTarget(inject1.JumpBackAddress);
        jumpBack1.WriteJump(codespace, JumpCondition::JMP);

        // instead of doing the previous array lookup for the ID, we check our bitfield, push the
        // counter if the bitfield says it's okay, or skip the check completely if the bitfield says
        // it's not okay
        auto inject2 =
            InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, pushIdHookAddress, codespace);

        // move relevant byte of bitfield into eax
        Emit_MOV_R32_R32(codespace, R32::EAX, R32::ESI);
        WriteInstruction24(codespace, 0xc1e803); // shr eax,3
        WriteInstruction24(codespace, 0x0fb680); // movzx eax,byte ptr[validDlcBitfield + eax]
        std::memcpy(codespace, &validDlcBitfield, 4);
        codespace += 4;

        // check correct bit
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::ESI);
        WriteInstruction24(codespace, 0x83e107);     // and ecx,7
        WriteInstruction40(codespace, 0xba01000000); // mov edx,1
        WriteInstruction16(codespace, 0xd3e2);       // shl edx,cl
        Emit_TEST_R32_R32(codespace, R32::EAX, R32::EDX);

        skipDlcCheck.WriteJump(codespace, JumpCondition::JZ);
        Emit_PUSH_R32(codespace, R32::ESI);

        BranchHelper4Byte jumpBack2;
        jumpBack2.SetTarget(inject2.JumpBackAddress);
        jumpBack2.WriteJump(codespace, JumpCondition::JMP);

        execData.Codespace = codespace;
    }
}
} // namespace SenLib::TX
