#include "exe_patch.h"

#include <cassert>
#include <cstring>

#include "util/memread.h"
#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace SenLib::Sen5 {
// reverie DLC IDs are very straightforward, all IDs from 0 to 155 are used, the rest isn't.
// so we can just test >= 156.
static constexpr uint16_t s_InternalDlcIds = 156;

void PatchCustomDlcMultiuse(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddressForUsage = GetCodeAddressEn(version, textRegion, 0x140208682);
    char* const injectAddressForShowingDisabledItem =
        GetCodeAddressEn(version, textRegion, 0x14068242e);

    char* codespace = execData.Codespace;

    {
        const auto injectResult =
            InjectJumpIntoCode<12>(logger, injectAddressForUsage, R64::RDX, codespace);
        std::memcpy(codespace,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        codespace += injectResult.OverwrittenInstructions.size();

        WriteInstruction24(codespace, 0x4181f8); // cmp r8d,s_InternalDlcIds
        HyoutaUtils::MemWrite::WriteAdvUInt32(codespace, static_cast<uint32_t>(s_InternalDlcIds));

        // if out of range it's custom
        BranchHelper1Byte isCustom;
        isCustom.WriteJump(codespace, JumpCondition::JAE);

        // not custom
        Emit_MOV_R64_IMM64(
            codespace, R64::RDX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RDX);

        // follow the conditional jump and always allow usage
        isCustom.SetTarget(codespace);
        int32_t offset = static_cast<int32_t>(
            HyoutaUtils::MemRead::ReadUInt32(injectResult.JumpBackAddress + 5));
        char* target = injectResult.JumpBackAddress + 5 + 4 + offset;
        Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(target));
        Emit_JMP_R64(codespace, R64::RDX);
    }

    {
        const auto injectResult = InjectJumpIntoCode<19, PaddingInstruction::Nop>(
            logger, injectAddressForShowingDisabledItem, R64::RCX, codespace);

        WriteInstruction8(codespace, 0x3d); // cmp eax,s_InternalDlcIds
        HyoutaUtils::MemWrite::WriteAdvUInt32(codespace, static_cast<uint32_t>(s_InternalDlcIds));

        // if out of range it's custom
        BranchHelper1Byte isCustom;
        isCustom.WriteJump(codespace, JumpCondition::JAE);

        // not custom
        std::memcpy(codespace,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        codespace += injectResult.OverwrittenInstructions.size();

        // in the custom case we just skip the overwritten code (which ends up setting or clearing
        // the carry flag depending on whether the DLC has been used or not), this works out because
        // the cmp instruction has already cleared the carry flag so the following cmovc does the
        // right thing. very sneaky.
        isCustom.SetTarget(codespace);

        Emit_MOV_R64_IMM64(
            codespace, R64::RCX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RCX);
    }

    execData.Codespace = codespace;
}

void PatchCustomDlcIfNotInParty(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressEn(version, textRegion, 0x140208752);

    char* codespace = execData.Codespace;

    {
        const auto injectResult =
            InjectJumpIntoCode<12>(logger, injectAddress, R64::RDX, codespace);
        std::memcpy(codespace,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        codespace += injectResult.OverwrittenInstructions.size();

        WriteInstruction40(codespace, 0x410fb71424); // movzx edx,word ptr[r12]
        WriteInstruction16(codespace, 0x81fa);       // cmp edx,s_InternalDlcIds
        HyoutaUtils::MemWrite::WriteAdvUInt32(codespace, static_cast<uint32_t>(s_InternalDlcIds));

        // if out of range it's custom
        BranchHelper1Byte isNotCustom;
        isNotCustom.WriteJump(codespace, JumpCondition::JNAE);

        // is custom, so force the check to succeed
        Emit_XOR_R64_R64(codespace, R64::RAX, R64::RAX);

        isNotCustom.SetTarget(codespace);
        Emit_MOV_R64_IMM64(
            codespace, R64::RDX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
        Emit_JMP_R64(codespace, R64::RDX);
    }

    execData.Codespace = codespace;
}

} // namespace SenLib::Sen5
