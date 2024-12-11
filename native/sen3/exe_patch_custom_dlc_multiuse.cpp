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
consteval std::array<uint32_t, 7> MakeInternalDlcBitfield() {
    const std::vector<uint16_t> internalIds = {{
        0,   1,   2,   3,   7,   8,   10,  11,  20,  21,  22,  23,  24,  25,  26,  60,
        61,  65,  70,  71,  72,  75,  76,  77,  78,  79,  85,  86,  87,  90,  91,  92,
        95,  96,  97,  100, 105, 106, 107, 108, 115, 116, 120, 121, 125, 126, 127, 130,
        131, 132, 135, 136, 140, 141, 142, 145, 146, 147, 148, 149, 150, 155, 156, 157,
        160, 161, 162, 165, 166, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
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

    using namespace SenPatcher::x64;

    char* const injectAddress = GetCodeAddressJpEn(version, textRegion, 0x1403080f0, 0x140311180);

    char* codespace = execData.Codespace;
    const auto injectResult =
        InjectJumpIntoCode<15, PaddingInstruction::Nop>(logger, injectAddress, R64::RAX, codespace);

    WriteInstruction24(codespace, 0x6681fa); // cmp dx,(s_InternalDlcBitfield.size() * 32)
    HyoutaUtils::MemWrite::WriteAdvUInt16(codespace,
                                          static_cast<uint16_t>(s_InternalDlcBitfield.size() * 32));
    // if out of range it's custom, so return unused
    BranchHelper1Byte returnUnused;
    returnUnused.WriteJump(codespace, JumpCondition::JAE);

    // this sets rax to the array index and r8d to the bit index
    std::memcpy(codespace,
                injectResult.OverwrittenInstructions.data(),
                injectResult.OverwrittenInstructions.size());
    codespace += injectResult.OverwrittenInstructions.size();

    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(s_InternalDlcBitfield.data()));
    WriteInstruction24(codespace, 0x8b1482);   // mov edx,dword ptr[rdx + rax*4]
    WriteInstruction32(codespace, 0x440fa3c2); // bt edx,r8d
    BranchHelper1Byte continueFunction;
    continueFunction.WriteJump(codespace, JumpCondition::JC); // if internal ID, continue the check

    returnUnused.SetTarget(codespace);
    Emit_XOR_R64_R64(codespace, R64::RAX, R64::RAX);
    Emit_RET(codespace);

    continueFunction.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RDX, std::bit_cast<uint64_t>(injectResult.JumpBackAddress));
    Emit_JMP_R64(codespace, R64::RDX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen3
