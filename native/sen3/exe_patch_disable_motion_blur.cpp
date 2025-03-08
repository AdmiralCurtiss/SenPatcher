#include "exe_patch.h"

#include <cassert>

#include "x64/emitter.h"
#include "x64/inject_jump_into.h"
#include "x64/page_unprotect.h"

namespace SenLib::Sen3 {
void PatchDisableMotionBlur(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;

    using namespace SenPatcher::x64;
    char* entryPoint = GetCodeAddressJpEn(version,
                                          textRegion,
                                          Addresses{.Jp106 = 0x140320b25,
                                                    .En106 = 0x14032a025,
                                                    .Jp107 = 0x140320aa5,
                                                    .En107 = 0x140329fa5});
    char* exitPoint = GetCodeAddressJpEn(version,
                                         textRegion,
                                         Addresses{.Jp106 = 0x140320c72,
                                                   .En106 = 0x14032a187,
                                                   .Jp107 = 0x140320bf2,
                                                   .En107 = 0x14032a107});

    char* codespace = execData.Codespace;
    auto injectResult = InjectJumpIntoCode<12>(logger, entryPoint, R64::RAX, codespace);
    char* inject = injectResult.JumpBackAddress;
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();

    WriteInstruction32(codespace, 0x6683fa01); // cmp dx,1

    BranchHelper1Byte shouldSkip;
    shouldSkip.WriteJump(codespace, JumpCondition::JE);

    // not skipping, continue with regular function
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(inject));
    Emit_JMP_R64(codespace, R64::RAX);

    // skipping, jump to epilogue
    shouldSkip.SetTarget(codespace);
    Emit_MOV_R64_IMM64(codespace, R64::RAX, std::bit_cast<uint64_t>(exitPoint));
    Emit_JMP_R64(codespace, R64::RAX);

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen3
