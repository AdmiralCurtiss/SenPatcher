#include "exe_patch.h"

#include <cassert>
#include <cstring>

#include "util/memwrite.h"
#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace SenLib::Sen4 {
consteval std::array<uint32_t, 7> MakeInternalDlcBitfield() {
    const std::vector<uint16_t> internalIds = {{
        1,   2,   3,   4,   5,   6,   7,   8,   9,   11,  13,  80,  90,  91,  92,  93,
        94,  95,  96,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
        113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
        129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 190, 191, 192, 193, 194, 195,
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

    char* const injectAddress = GetCodeAddressJpEn(version, textRegion, 0x1402eeae0, 0x1402f1070);

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
} // namespace SenLib::Sen4
