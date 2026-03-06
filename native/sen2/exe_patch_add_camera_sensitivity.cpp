#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
static int s_Sensitivity = 0;
static constexpr float s_float0p5 = 0.5f;
static constexpr float s_float1 = 1.0f;
static constexpr float s_float3 = 3.0f;
static constexpr float s_float7 = 7.0f;

void PatchAddCameraSensitivity(PatchExecData& execData, int sensitivity) {
    s_Sensitivity = sensitivity < 0 ? 3 : sensitivity; // 3 is equivalent to the default speed

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;
    char* addressInjectPos1 = GetCodeAddressJpEn(version, textRegion, 0x4d7368, 0x4d75a8);
    char* addressInjectPos2 = GetCodeAddressJpEn(version, textRegion, 0x4d7392, 0x4d75d2);

    using JC = JumpCondition;

    for (char* address : {addressInjectPos1, addressInjectPos2}) {
        BranchHelper4Byte jumpBack;
        char* tmp = codespace;
        const auto injectResult = InjectJumpIntoCode<5>(logger, address, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // mulss xmm0,dword ptr[ebp+0x10]
        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        // eax is free
        // xmm1, xmm2 should be usable
        //
        // what we're doing here is calculating
        // (s_Sensitivity < 3) ? std::lerp(0.5f, 1.0f, static_cast<float>(s_Sensitivity) / 3.0f)
        //                     : std::lerp(1.0f, 2.0f, static_cast<float>(s_Sensitivity - 3) / 7.0f)
        // and then multiplying that onto xmm0
        // the resulting behavior should be identical to CS3's in-game camera sensitivity slider

        BranchHelper1Byte rangeDone;
        Emit_MOV_R32_DwordPtr(tmp, R32::EAX, &s_Sensitivity);
        Emit_CMP_R32_IMM32(tmp, R32::EAX, 3);
        BranchHelper1Byte lowRange;
        lowRange.WriteJump(tmp, JC::JB);

        // high range
        // 1.0 + ((eax-3)/7)
        Emit_SUB_R32_IMM32(tmp, R32::EAX, 3);
        Emit_CVTSI2SS_XMM_R32(tmp, XMM::XMM1, R32::EAX);
        Emit_DIVSS_XMM_DwordPtr(tmp, XMM::XMM1, &s_float7); // /= 7.0
        Emit_ADDSS_XMM_DwordPtr(tmp, XMM::XMM1, &s_float1); // += 1.0
        rangeDone.WriteJump(tmp, JC::JMP);

        // low range
        // 0.5 + (eax/3) * 0.5
        lowRange.SetTarget(tmp);
        Emit_CVTSI2SS_XMM_R32(tmp, XMM::XMM1, R32::EAX);
        Emit_DIVSS_XMM_DwordPtr(tmp, XMM::XMM1, &s_float3);   // /= 3.0
        Emit_MULSS_XMM_DwordPtr(tmp, XMM::XMM1, &s_float0p5); // *= 0.5
        Emit_ADDSS_XMM_DwordPtr(tmp, XMM::XMM1, &s_float0p5); // += 0.5

        rangeDone.SetTarget(tmp);
        Emit_MULSS_XMM_XMM(tmp, XMM::XMM0, XMM::XMM1);

        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack
        codespace = tmp;
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen2
