#include "exe_patch.h"

#include <cassert>
#include <cstring>

#include "util/memwrite.h"
#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
consteval std::array<uint32_t, 7> MakeInternalDlcBitfield() {
    const std::vector<uint16_t> internalIds = {{
        0,   1,   2,   3,   4,   5,   6,   20,  21,  22,  23,  24,  30,  31,  32,  40,
        41,  42,  50,  51,  52,  53,  54,  60,  61,  62,  70,  71,  72,  73,  74,  75,
        76,  77,  78,  79,  80,  81,  82,  100, 101, 102, 103, 110, 111, 112, 120, 121,
        122, 130, 131, 132, 140, 141, 142, 150, 151, 152, 160, 161, 170, 171, 172, 173,
        176, 177, 200, 201, 202, 203, 210, 211, 212, 213, 214, 215, 216,
    }};
    std::array<uint32_t, 7> bitfield{};
    for (size_t i = 0; i < internalIds.size(); ++i) {
        const uint16_t dlc_id = internalIds[i];
        bitfield[dlc_id >> 5] |= (static_cast<uint32_t>(1) << static_cast<int>(dlc_id & 0x1f));
    }
    return bitfield;
}
static constexpr std::array<uint32_t, 7> s_InternalDlcBitfield = MakeInternalDlcBitfield();

void PatchCustomDlcMultiuse(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;

    char* const injectAddress = GetCodeAddressSteamGog(version, textRegion, 0x580200, 0x57e7a0);

    char* codespace = execData.Codespace;
    const auto injectResult =
        InjectJumpIntoCode<7, PaddingInstruction::Nop>(logger, injectAddress, codespace);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);

    WriteInstruction24(codespace, 0x6681fa); // cmp dx,(s_InternalDlcBitfield.size() * 32)
    HyoutaUtils::MemWrite::WriteAdvUInt16(codespace,
                                          static_cast<uint16_t>(s_InternalDlcBitfield.size() * 32));
    // if out of range it's custom, so return unused
    BranchHelper1Byte returnUnused;
    returnUnused.WriteJump(codespace, JumpCondition::JAE);

    Emit_PUSH_R32(codespace, R32::EDX);
    Emit_PUSH_R32(codespace, R32::ECX);

    // this sets eax to the array index and edx to the bit index
    Emit_MOV_R32_R32(codespace, R32::EAX, R32::EDX);
    WriteInstruction24(codespace, 0xc1e805); // shr eax,5
    WriteInstruction24(codespace, 0x83e21f); // and edx,1fh

    Emit_MOV_R32_IMM32(codespace, R32::ECX, std::bit_cast<uint32_t>(s_InternalDlcBitfield.data()));
    WriteInstruction24(codespace, 0x8b0481); // mov eax,dword ptr[ecx + eax*4]
    WriteInstruction24(codespace, 0x0fa3d0); // bt eax,edx

    Emit_POP_R32(codespace, R32::ECX);
    Emit_POP_R32(codespace, R32::EDX);

    BranchHelper1Byte continueFunction;
    continueFunction.WriteJump(codespace, JumpCondition::JC); // if internal ID, continue the check

    returnUnused.SetTarget(codespace);
    Emit_XOR_R32_R32(codespace, R32::EAX, R32::EAX);
    Emit_POP_R32(codespace, R32::EBP);
    Emit_RET_IMM16(codespace, 4);

    continueFunction.SetTarget(codespace);
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    execData.Codespace = codespace;
}

void PatchCustomDlcAllow(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;

    char* const injectAddress = GetCodeAddressSteamGog(version, textRegion, 0x5f385c, 0x5f1ced);

    char* codespace = execData.Codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, injectAddress, codespace);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    WriteInstruction24(codespace, 0x0fb716); // movzx edx,word ptr[esi]
    WriteInstruction24(codespace, 0x6681fa); // cmp dx,(s_InternalDlcBitfield.size() * 32)
    HyoutaUtils::MemWrite::WriteAdvUInt16(codespace,
                                          static_cast<uint16_t>(s_InternalDlcBitfield.size() * 32));
    // if out of range it's custom
    BranchHelper1Byte isCustom;
    isCustom.WriteJump(codespace, JumpCondition::JAE);

    // on Steam we need to preserve eax here
    if (version == GameVersion::Steam) {
        Emit_PUSH_R32(codespace, R32::EAX);
    }

    // this sets eax to the array index and edx to the bit index
    Emit_MOV_R32_R32(codespace, R32::EAX, R32::EDX);
    WriteInstruction24(codespace, 0xc1e805); // shr eax,5
    WriteInstruction24(codespace, 0x83e21f); // and edx,1fh

    Emit_MOV_R32_IMM32(codespace, R32::ECX, std::bit_cast<uint32_t>(s_InternalDlcBitfield.data()));
    WriteInstruction24(codespace, 0x8b0481); // mov eax,dword ptr[ecx + eax*4]
    WriteInstruction24(codespace, 0x0fa3d0); // bt eax,edx
    isCustom.WriteJump(codespace, JumpCondition::JNC);

    // not custom, continue with regular code
    if (version == GameVersion::Steam) {
        Emit_POP_R32(codespace, R32::EAX);
    }
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // on custom, skip the call and set al nonzero for the next check
    isCustom.SetTarget(codespace);

    // fix up stack for the call we just interrupted
    if (version == GameVersion::Steam) {
        Emit_POP_R32(codespace, R32::EAX);
    }
    Emit_POP_R32(codespace, R32::ECX);

    WriteInstruction16(codespace, 0xb001); // mov al,1
    BranchHelper4Byte jump_back_skip;
    jump_back_skip.SetTarget(injectResult.JumpBackAddress + 5);
    jump_back_skip.WriteJump(codespace, JumpCondition::JMP);


    execData.Codespace = codespace;
}
} // namespace SenLib::TX
