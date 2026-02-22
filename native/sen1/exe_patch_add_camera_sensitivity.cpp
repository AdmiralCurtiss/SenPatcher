#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen1 {
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
    char* addressInjectPos = GetCodeAddressJpEn(version, textRegion, 0x53b154, 0x53c874);

    using JC = JumpCondition;

    {
        BranchHelper4Byte jumpBack;
        char* tmp = codespace;
        const auto injectResult = InjectJumpIntoCode<6>(logger, addressInjectPos, tmp);
        jumpBack.SetTarget(injectResult.JumpBackAddress);

        // fld 30.0
        std::memcpy(tmp,
                    injectResult.OverwrittenInstructions.data(),
                    injectResult.OverwrittenInstructions.size());
        tmp += injectResult.OverwrittenInstructions.size();

        // eax is free
        // three fpu registers are free
        // [ebp - 4] is free

        // what we're doing here is calculating
        // (s_Sensitivity < 3) ? std::lerp(0.5f, 1.0f, static_cast<float>(s_Sensitivity) / 3.0f)
        //                     : std::lerp(1.0f, 2.0f, static_cast<float>(s_Sensitivity - 3) / 7.0f)
        // and then multiplying that onto the fld 30.0 we injected over
        // the resulting behavior should be identical to CS3's in-game camera sensitivity slider

        BranchHelper1Byte rangeDone;
        Emit_MOV_R32_DwordPtr(tmp, R32::EAX, &s_Sensitivity);
        Emit_CMP_R32_IMM32(tmp, R32::EAX, 3);
        BranchHelper1Byte lowRange;
        lowRange.WriteJump(tmp, JC::JB);

        // high range
        // 1.0 + ((eax-3)/7)
        Emit_SUB_R32_IMM32(tmp, R32::EAX, 3);
        Emit_MOV_DwordPtrR32PlusOffset_R32(tmp, R32::EBP, -4, R32::EAX);
        Emit_FILD_DwordPtrR32PlusOffset(tmp, R32::EBP, -4); // push st0 = (eax-3)
        Emit_FDIV_DwordPtr(tmp, &s_float7);                 // st0 /= 7.0
        Emit_FADD_DwordPtr(tmp, &s_float1);                 // st0 += 1.0
        rangeDone.WriteJump(tmp, JC::JMP);

        // low range
        // 0.5 + (eax/3) * 0.5
        lowRange.SetTarget(tmp);
        Emit_MOV_DwordPtrR32PlusOffset_R32(tmp, R32::EBP, -4, R32::EAX);
        Emit_FILD_DwordPtrR32PlusOffset(tmp, R32::EBP, -4); // push st0 = eax
        Emit_FDIV_DwordPtr(tmp, &s_float3);                 // st0 /= 3.0
        Emit_FMUL_DwordPtr(tmp, &s_float0p5);               // st0 *= 0.5
        Emit_FADD_DwordPtr(tmp, &s_float0p5);               // st0 += 0.5

        rangeDone.SetTarget(tmp);
        WriteInstruction16(tmp, 0xdec9); // fmulp st1,st0

        jumpBack.WriteJump(tmp, JC::JMP); // jmp jumpBack
        codespace = tmp;
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen1
