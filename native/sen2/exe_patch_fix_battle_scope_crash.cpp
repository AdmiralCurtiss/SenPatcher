#include "exe_patch.h"

#include <cassert>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::Sen2 {
void PatchAddNullCheckBattleScopeCrashMaybe(PatchExecData& execData) {
    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;

    using namespace SenPatcher::x86;

    // Multiple reported crashes at this address while using a Battle Scope.
    char* const checkAddress = GetCodeAddressJpEn(version, textRegion, 0x52e53e, 0x52e1ce);

    // These constructs in the area also deref this pointer, so check those too just in case...
    char* const checkAddress2 = GetCodeAddressJpEn(version, textRegion, 0x52e500, 0x52e190) - 6;
    char* const checkAddress3 = GetCodeAddressJpEn(version, textRegion, 0x52e513, 0x52e1a3) - 6;

    // After the above patches and manually forcing the relevant pointer to 0, this function
    // crashed, so let's patch this too. tbqh I don't think this is the right approach, but I don't
    // know what actually causes the situation in the first place, and it *seems* to work, so...
    char* const checkAddress4 = GetCodeAddressJpEn(version, textRegion, 0x52b037, 0x52acc7);

    // clang-format off
    using JC = JumpCondition;

    // Battle Scope crash: Null check 1
    char* exitFunction;
    {
        char* tmp = codespace;
        BranchHelper4Byte jmp_to_code;
        BranchHelper4Byte jmp_back;
        BranchHelper1Byte skip_deref;
        jmp_to_code.SetTarget(tmp);

        // this is the instruction that crashes when eax == 0
        std::array<char, 3> overwrittenInstruction1; // cmp dl,byte ptr[eax+0x3d]
        std::memcpy(overwrittenInstruction1.data(), checkAddress, overwrittenInstruction1.size());

        // this should be executed regardless
        std::array<char, 2> overwrittenInstruction2; // mov edx,dword ptr[ecx]
        std::memcpy(overwrittenInstruction2.data(), checkAddress + 3, overwrittenInstruction2.size());

        jmp_back.SetTarget(checkAddress + 5);
        int8_t jumpOffset = static_cast<int8_t>(*(checkAddress + 6)); // jge +0x20
        exitFunction = checkAddress + 7 + jumpOffset;

        {
            char* tmp2 = checkAddress;
            PageUnprotect page(logger, tmp2, 5);
            jmp_to_code.WriteJump(tmp2, JC::JMP); // jmp to our code
        }

        Emit_TEST_R32_R32(tmp, R32::EAX, R32::EAX);  // test eax,eax
        skip_deref.WriteJump(tmp, JC::JZ);           // jz skip_deref
        std::memcpy(tmp, overwrittenInstruction1.data(), overwrittenInstruction1.size());
        tmp += overwrittenInstruction1.size();
        skip_deref.SetTarget(tmp);
        std::memcpy(tmp, overwrittenInstruction2.data(), overwrittenInstruction2.size());
        tmp += overwrittenInstruction2.size();
        jmp_back.WriteJump(tmp, JC::JMP);  // jmp back

        // why this works: by skipping overwrittenInstruction1, the zero flag keeps the state of the test eax,eax
        // this causes the next jge in the original code to return the function early, without us having to explicitly do that

        codespace = tmp;
    }

    // Battle Scope crash: Null check 2
    {
        char* tmp = codespace;

        BranchHelper4Byte exit_function;
        BranchHelper1Byte skip_deref;
        exit_function.SetTarget(exitFunction);

        const auto injectResult2 = InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, checkAddress2, tmp);
        BranchHelper4Byte jmp_back_2;
        jmp_back_2.SetTarget(injectResult2.JumpBackAddress);

        std::memcpy(tmp, injectResult2.OverwrittenInstructions.data(), injectResult2.OverwrittenInstructions.size());
        tmp += injectResult2.OverwrittenInstructions.size();
        Emit_TEST_R32_R32(tmp, R32::EAX, R32::EAX);     // test eax,eax
        skip_deref.WriteJump(tmp, JC::JZ);              // jz skip_deref
        jmp_back_2.WriteJump(tmp, JC::JMP);             // jmp jmp_back_2

        const auto injectResult3 = InjectJumpIntoCode<6, PaddingInstruction::Nop>(logger, checkAddress3, tmp);
        BranchHelper4Byte jmp_back_3;
        jmp_back_3.SetTarget(injectResult3.JumpBackAddress);

        std::memcpy(tmp, injectResult3.OverwrittenInstructions.data(), injectResult3.OverwrittenInstructions.size());
        tmp += injectResult3.OverwrittenInstructions.size();
        Emit_TEST_R32_R32(tmp, R32::EAX, R32::EAX);     // test eax,eax
        skip_deref.WriteJump(tmp, JC::JZ);              // jz skip_deref
        jmp_back_3.WriteJump(tmp, JC::JMP);             // jmp jmp_back_3

        skip_deref.SetTarget(tmp);
        exit_function.WriteJump(tmp, JC::JMP); // jmp back

        codespace = tmp;
    }

    // Battle Scope crash: Null check 3
    {
        char* tmp = codespace;

        const auto injectResult4 = InjectJumpIntoCode<5, PaddingInstruction::Nop>(logger, checkAddress4, tmp);

        int32_t jumpOffset;
        std::memcpy(&jumpOffset, checkAddress4 + 5 + 7 + 2, 4); // grabs the jnz offset
        char* const exitFunction4 = checkAddress4 + 5 + 7 + 2 + 4 + jumpOffset;

        BranchHelper4Byte jmp_back;
        BranchHelper4Byte exit_function;
        jmp_back.SetTarget(injectResult4.JumpBackAddress);
        exit_function.SetTarget(exitFunction4);

        std::memcpy(tmp, injectResult4.OverwrittenInstructions.data(), injectResult4.OverwrittenInstructions.size());
        tmp += injectResult4.OverwrittenInstructions.size();

        WriteInstruction16(tmp, 0x83be);     // cmp dword ptr[esi+03ech],0
        const int32_t structOffset = 0x3ec;
        std::memcpy(tmp, &structOffset, 4);
        tmp += 4;
        *tmp = 0;
        ++tmp;
        exit_function.WriteJump(tmp, JC::JE); // je exit_function
        jmp_back.WriteJump(tmp, JC::JMP);     // jmp back

        codespace = tmp;
    }
    // clang-format on

    execData.Codespace = codespace;
}
} // namespace SenLib::Sen2
