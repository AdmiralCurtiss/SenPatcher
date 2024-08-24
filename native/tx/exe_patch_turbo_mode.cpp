#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

namespace SenLib::TX {
static bool TurboActive = false;
static bool TurboIsToggle = false;
static bool TurboButtonPressedLastFrame = false;
static float TurboFactor = 0.0f;
static float RealTimeStep = 0.0f;
static float TempStoreMul = 0.0f;

namespace {
struct TimestepCounterStruct {
    float Unknown1;
    float Unknown2;
    float FixedTimeStep;
    float Unknown4;
    uint32_t Unknown5;
    uint32_t Unknown6;
    float Unknown7;
    uint32_t Unknown8;
    bool Unknown9;
    char Padding[3];
    uint32_t Unknown10;
};
static_assert(offsetof(TimestepCounterStruct, Unknown1) == 0x00);
static_assert(offsetof(TimestepCounterStruct, Unknown2) == 0x04);
static_assert(offsetof(TimestepCounterStruct, FixedTimeStep) == 0x08);
static_assert(offsetof(TimestepCounterStruct, Unknown4) == 0x0c);
static_assert(offsetof(TimestepCounterStruct, Unknown5) == 0x10);
static_assert(offsetof(TimestepCounterStruct, Unknown6) == 0x14);
static_assert(offsetof(TimestepCounterStruct, Unknown7) == 0x18);
static_assert(offsetof(TimestepCounterStruct, Unknown8) == 0x1c);
static_assert(offsetof(TimestepCounterStruct, Unknown9) == 0x20);
static_assert(offsetof(TimestepCounterStruct, Unknown10) == 0x24);
} // namespace

static void __fastcall HandleTurbo(float* timestep,
                                   TimestepCounterStruct* counters,
                                   int buttonHeldRegister) {
    const bool buttonHeld = (buttonHeldRegister & 0xff) != 0;
    bool turboActiveThisFrame = false;
    if (TurboIsToggle) {
        if (buttonHeld && !TurboButtonPressedLastFrame) {
            TurboButtonPressedLastFrame = true;
            turboActiveThisFrame = !TurboActive;
            TurboActive = turboActiveThisFrame;
        } else {
            TurboButtonPressedLastFrame = buttonHeld;
            turboActiveThisFrame = TurboActive;
        }
    } else {
        turboActiveThisFrame = buttonHeld;
        TurboActive = turboActiveThisFrame;
    }

    // Tokyo Xanadu doesn't actually implement a variable timestep. Instead, it just measures time
    // and runs a fixed timestep every time a threshold is crossed. This is very unfortunate for
    // several reasons, especially since the underlying engine should support variable timesteps
    // mostly fine (since the CS games do). I suspect this is also the cause of the game's
    // microstuttering, sometimes you just get 0 or 2 frame advancements for a rendered frame.
    if (turboActiveThisFrame) {
        // Technically incorrect (turbo can be toggled between fixed timestep advances), but should
        // work okay enough given the circumstances.
        RealTimeStep = counters->FixedTimeStep / TurboFactor;
    } else {
        RealTimeStep = counters->FixedTimeStep;
    }

    if (turboActiveThisFrame) {
        *timestep = (*timestep * TurboFactor);
    }

    return;
}

// static void __fastcall DebugFunc(void* stack) {
//     int* buffer = *(int**)(((char*)stack) - 0x18);
//     float* floatbuffer = (float*)buffer;
//
//     if (buffer[0x1ec / 4] == 0x3eb44d4a) {
//         __debugbreak();
//     }
//
//     return;
// }

static const char EN_ButtonName_MenuAction1[] = "Menu - Action 1";
static const char JP_ButtonName_MenuAction1[] =
    "\xE3\x83\xA1\xE3\x83\x8B\xE3\x83\xA5\xE3\x83\xBC\xE3\x83\xBB"
    "Action 1";
static const char EN_ButtonName_MenuAction2[] = "Menu - Action 2";
static const char JP_ButtonName_MenuAction2[] =
    "\xE3\x83\xA1\xE3\x83\x8B\xE3\x83\xA5\xE3\x83\xBC\xE3\x83\xBB"
    "Action 2";
static const char EN_ButtonName_MenuZoomIn[] = "Menu - Zoom In";
static const char JP_ButtonName_MenuZoomIn[] =
    "\xE3\x83\xA1\xE3\x83\x8B\xE3\x83\xA5\xE3\x83\xBC\xE3\x83\xBB"
    "\xE6\x8B\xA1\xE5\xA4\xA7";
static const char EN_ButtonName_MenuZoomOut[] = "Menu - Zoom Out";
static const char JP_ButtonName_MenuZoomOut[] =
    "\xE3\x83\xA1\xE3\x83\x8B\xE3\x83\xA5\xE3\x83\xBC\xE3\x83\xBB"
    "\xE7\xB8\xAE\xE5\xB0\x8F";
static const char EN_ButtonName_TurboMode[] = "Turbo Mode";
static const char JP_ButtonName_TurboMode[] =
    "\xE3\x82\xBF\xE3\x83\xBC\xE3\x83\x9C\xE3\x83\xA2\xE3\x83\xBC\xE3\x83\x89";
static const char EN_ButtonName_FishingUp[] = "Fishing - Up";
static const char JP_ButtonName_FishingUp[] = "Fishing\xE3\x83\xBB\xE4\xB8\x8A";
static const char EN_ButtonName_FishingDown[] = "Fishing - Down";
static const char JP_ButtonName_FishingDown[] = "Fishing\xE3\x83\xBB\xE4\xB8\x8B";
static const char EN_ButtonName_FishingLeft[] = "Fishing - Left";
static const char JP_ButtonName_FishingLeft[] = "Fishing\xE3\x83\xBB\xE5\xB7\xA6";
static const char EN_ButtonName_FishingRight[] = "Fishing - Right";
static const char JP_ButtonName_FishingRight[] = "Fishing\xE3\x83\xBB\xE5\x8F\xB3";
static constexpr uint32_t Index_MenuAction1 = 0x25 + 0;
static constexpr uint32_t Index_MenuAction2 = 0x25 + 1;
static constexpr uint32_t Index_MenuZoomIn = 0x25 + 2;
static constexpr uint32_t Index_MenuZoomOut = 0x25 + 3;
static constexpr uint32_t Index_TurboMode = 0x25 + 4;
static constexpr uint32_t Index_FishingUp = 0x25 + 5;
static constexpr uint32_t Index_FishingDown = 0x25 + 6;
static constexpr uint32_t Index_FishingLeft = 0x25 + 7;
static constexpr uint32_t Index_FishingRight = 0x25 + 8;
static constexpr uint32_t ButtonsToAdd = 9;

void PatchTurboAndButtonMappings(PatchExecData& execData,
                                 bool makeToggle,
                                 float factor,
                                 bool useJapanese) {
    if (factor >= 1.0f && factor <= 6.0f) {
        TurboFactor = factor;
    } else {
        TurboFactor = 2.0f;
    }

    TurboIsToggle = makeToggle;

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    using namespace SenPatcher::x86;
    using JC = JumpCondition;

    // TODO: Actually make the new bindings work.
    // TODO: Add default bindings when launching without ini or resetting to default.

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x437521, 0x435d41);
    char* const ed8appPtr = GetCodeAddressSteamGog(version, textRegion, 0xb4d888, 0xb4c888);
    char* const readInputCase10 = GetCodeAddressSteamGog(version, textRegion, 0x43865c, 0x436e8c);
    char* const readInputCommonCall =
        GetCodeAddressSteamGog(version, textRegion, 0x43841f, 0x436c4f);
    char* const countButtonMappingsToAlloc =
        GetCodeAddressSteamGog(version, textRegion, 0x4058d2, 0x405412) + 1;
    char* const countButtonMappingsToInit =
        GetCodeAddressSteamGog(version, textRegion, 0x405940, 0x405480) + 2;
    char* const countButtonMappingsToWriteToIni =
        GetCodeAddressSteamGog(version, textRegion, 0x407393, 0x406ed3) + 2;
    char* const countButtonMappingsToReadFromIni =
        GetCodeAddressSteamGog(version, textRegion, 0x40724f, 0x406d8f) + 2;
    char* const getButtonMappingNameFunction =
        GetCodeAddressSteamGog(version, textRegion, 0x405770, 0x4052b0);

    // TODO: are all of these needed?
    char* const countButtonMappingsInRemappingGUI1 =
        GetCodeAddressSteamGog(version, textRegion, 0x67f2c4, 0x67d7b4) + 2;
    char* const countButtonMappingsInRemappingGUI2 =
        GetCodeAddressSteamGog(version, textRegion, 0x6754b3, 0x6739a3) + 2;
    char* const countButtonMappingsInRemappingGUI3 =
        GetCodeAddressSteamGog(version, textRegion, 0x6aa666, 0x6a8be6) + 2;
    char* const countButtonMappingsInRemappingGUI4 =
        GetCodeAddressSteamGog(version, textRegion, 0x675791, 0x673c81) + 2;
    char* const countButtonMappingsInRemappingGUI5 =
        GetCodeAddressSteamGog(version, textRegion, 0x67f411, 0x67d901) + 6;
    char* const countButtonMappingsInRemappingGUI6 =
        GetCodeAddressSteamGog(version, textRegion, 0x68f976, 0x68def6) + 1; // scroll bar

    char* const addressLoadTimeStepForLipflaps1 =
        GetCodeAddressSteamGog(version, textRegion, 0x536db0, 0x5353c0);
    char* const addressLoadTimeStepForLipflaps2 =
        GetCodeAddressSteamGog(version, textRegion, 0x536dd0, 0x5353e0);
    char* const addressLoadTimeStepForLipflaps3 =
        GetCodeAddressSteamGog(version, textRegion, 0x536e26, 0x535436);
    char* const addressLoadTimeStepForActiveVoice =
        GetCodeAddressSteamGog(version, textRegion, 0x5373f9, 0x535a09);
    float* const addrRealTimeStep = &RealTimeStep;
    float* const addrTempStoreMul = &TempStoreMul;

    // debug
    //{
    //    char* const aa = GetCodeAddressSteamGog(version, textRegion, 0, 0x76ec4b);
    //    auto injectResult = InjectJumpIntoCode<5>(logger, aa, codespace);
    //    BranchHelper4Byte jump_back_dbg;
    //    jump_back_dbg.SetTarget(injectResult.JumpBackAddress);
    //    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
    //    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    //    codespace += overwrittenInstructions.size();
    //
    //    Emit_PUSH_R32(codespace, R32::EAX);
    //    Emit_PUSH_R32(codespace, R32::EBX);
    //    Emit_PUSH_R32(codespace, R32::ECX);
    //    Emit_PUSH_R32(codespace, R32::EDX);
    //    Emit_PUSH_R32(codespace, R32::ESI);
    //    Emit_PUSH_R32(codespace, R32::EDI);
    //    Emit_PUSH_R32(codespace, R32::EBP);
    //
    //    BranchHelper4Byte call_dbg;
    //    void* debug_func_ptr = DebugFunc;
    //    call_dbg.SetTarget(static_cast<char*>(debug_func_ptr));
    //    Emit_MOV_R32_R32(codespace, R32::ECX, R32::EBP);
    //    call_dbg.WriteJump(codespace, JumpCondition::CALL);
    //
    //    Emit_POP_R32(codespace, R32::EBP);
    //    Emit_POP_R32(codespace, R32::EDI);
    //    Emit_POP_R32(codespace, R32::ESI);
    //    Emit_POP_R32(codespace, R32::EDX);
    //    Emit_POP_R32(codespace, R32::ECX);
    //    Emit_POP_R32(codespace, R32::EBX);
    //    Emit_POP_R32(codespace, R32::EAX);
    //
    //    jump_back_dbg.WriteJump(codespace, JumpCondition::JMP);
    //}


    // some extra notes:
    // - call at 0x673c07 positions the remapping UI table rows
    // - call at 0x67d7eb controls the size of the background window

    // add name strings for the new actions
    {
        // luckily, the two arrays that hold the JP and EN action names are right next to eachother
        // in memory, so we can treat them as a single long array of double the size. set this up.
        PageUnprotect page(logger, getButtonMappingNameFunction, 0x21);
        const char** enArrayLocation;
        std::memcpy(&enArrayLocation, getButtonMappingNameFunction + 0x1b, 4);
        static constexpr uint32_t actionNameArrayLength = 0x25;
        PageUnprotect page2(logger, enArrayLocation, actionNameArrayLength * sizeof(char*) * 2);

        if (useJapanese) {
            // copy the JP strings upwards over the EN strings
            std::memcpy(enArrayLocation,
                        enArrayLocation + actionNameArrayLength,
                        actionNameArrayLength * sizeof(char*));

            *(enArrayLocation + Index_MenuAction1) = JP_ButtonName_MenuAction1;
            *(enArrayLocation + Index_MenuAction2) = JP_ButtonName_MenuAction2;
            *(enArrayLocation + Index_MenuZoomIn) = JP_ButtonName_MenuZoomIn;
            *(enArrayLocation + Index_MenuZoomOut) = JP_ButtonName_MenuZoomOut;
            *(enArrayLocation + Index_TurboMode) = JP_ButtonName_TurboMode;
            *(enArrayLocation + Index_FishingUp) = JP_ButtonName_FishingUp;
            *(enArrayLocation + Index_FishingDown) = JP_ButtonName_FishingDown;
            *(enArrayLocation + Index_FishingLeft) = JP_ButtonName_FishingLeft;
            *(enArrayLocation + Index_FishingRight) = JP_ButtonName_FishingRight;
        } else {
            *(enArrayLocation + Index_MenuAction1) = EN_ButtonName_MenuAction1;
            *(enArrayLocation + Index_MenuAction2) = EN_ButtonName_MenuAction2;
            *(enArrayLocation + Index_MenuZoomIn) = EN_ButtonName_MenuZoomIn;
            *(enArrayLocation + Index_MenuZoomOut) = EN_ButtonName_MenuZoomOut;
            *(enArrayLocation + Index_TurboMode) = EN_ButtonName_TurboMode;
            *(enArrayLocation + Index_FishingUp) = EN_ButtonName_FishingUp;
            *(enArrayLocation + Index_FishingDown) = EN_ButtonName_FishingDown;
            *(enArrayLocation + Index_FishingLeft) = EN_ButtonName_FishingLeft;
            *(enArrayLocation + Index_FishingRight) = EN_ButtonName_FishingRight;
        }

        // always use the (now extended and possibly overwritten) EN strings
        std::memcpy(getButtonMappingNameFunction + 3, getButtonMappingNameFunction + 0xa, 3);
        std::memcpy(getButtonMappingNameFunction + 6, getButtonMappingNameFunction + 0x18, 9);
        std::memset(getButtonMappingNameFunction + 15, 0xcc, 0x21 - 15);
    }

    // increase the amount of button mappings
    {
        PageUnprotect page(logger, countButtonMappingsToAlloc, 1);
        (*countButtonMappingsToAlloc) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToInit, 1);
        (*countButtonMappingsToInit) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToWriteToIni, 1);
        (*countButtonMappingsToWriteToIni) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToReadFromIni, 1);
        (*countButtonMappingsToReadFromIni) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI1, 1);
        (*countButtonMappingsInRemappingGUI1) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI2, 1);
        (*countButtonMappingsInRemappingGUI2) += ButtonsToAdd;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI3, 4);
        uint32_t tmp;
        std::memcpy(&tmp, countButtonMappingsInRemappingGUI3, 4);
        tmp += (ButtonsToAdd * 4);
        std::memcpy(countButtonMappingsInRemappingGUI3, &tmp, 4);
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI4, 4);
        uint32_t tmp;
        std::memcpy(&tmp, countButtonMappingsInRemappingGUI4, 4);
        tmp += (ButtonsToAdd * 4);
        std::memcpy(countButtonMappingsInRemappingGUI4, &tmp, 4);
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI5, 4);
        float tmp;
        std::memcpy(&tmp, countButtonMappingsInRemappingGUI5, 4);
        tmp += static_cast<float>(ButtonsToAdd);
        std::memcpy(countButtonMappingsInRemappingGUI5, &tmp, 4);
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI6, 1);
        (*countButtonMappingsInRemappingGUI6) += ButtonsToAdd;
    }

    // use the unscaled timestep for lipflaps
    // this is a three-step process as the timestep is transformed twice before being passed to the
    // function that actually applies the time advancement
    {
        char* codespace = execData.Codespace;
        const auto inject =
            InjectJumpIntoCode<5>(logger, addressLoadTimeStepForLipflaps1, codespace);

        WriteInstruction32(codespace, 0xf30f100d); // movss xmm1,dword ptr[&RealTimeStep]
        std::memcpy(codespace, &addrRealTimeStep, 4);
        codespace += 4;
        WriteInstruction32(codespace, 0xf30f59c8); // mulss xmm1,xmm0
        WriteInstruction32(codespace, 0xf30f110d); // movss dword ptr[&TempStoreMul],xmm1
        std::memcpy(codespace, &addrTempStoreMul, 4);
        codespace += 4;

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        BranchHelper4Byte jmpBack;
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }
    {
        char* codespace = execData.Codespace;
        const auto inject =
            InjectJumpIntoCode<5>(logger, addressLoadTimeStepForLipflaps2, codespace);

        WriteInstruction32(codespace, 0xf30f100d); // movss xmm1,dword ptr[&TempStoreMul]
        std::memcpy(codespace, &addrTempStoreMul, 4);
        codespace += 4;
        WriteInstruction32(codespace, 0xf30f59c8); // mulss xmm1,xmm0
        WriteInstruction32(codespace, 0xf30f110d); // movss dword ptr[&TempStoreMul],xmm1
        std::memcpy(codespace, &addrTempStoreMul, 4);
        codespace += 4;

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        BranchHelper4Byte jmpBack;
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }
    {
        char* codespace = execData.Codespace;
        const auto inject =
            InjectJumpIntoCode<5>(logger, addressLoadTimeStepForLipflaps3, codespace);

        WriteInstruction32(codespace, 0xf30f1005); // movss xmm0,dword ptr[&TempStoreMul]
        std::memcpy(codespace, &addrTempStoreMul, 4);
        codespace += 4;

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        BranchHelper4Byte jmpBack;
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }

    // use unscaled timestep for active voices
    {
        char* codespace = execData.Codespace;
        const auto inject =
            InjectJumpIntoCode<5>(logger, addressLoadTimeStepForActiveVoice, codespace);

        WriteInstruction32(codespace, 0xf30f1005); // movss xmm0,dword ptr[&RealTimeStep]
        std::memcpy(codespace, &addrRealTimeStep, 4);
        codespace += 4;

        BranchHelper4Byte jmpBack;
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }


    // then, inject the turbo mode code into the function that handles the per-frame timestep
    char* codespace = execData.Codespace;
    char* codespaceBegin = codespace;
    auto injectResult = InjectJumpIntoCode<5>(logger, entryPoint, codespaceBegin);
    BranchHelper4Byte jump_back;
    jump_back.SetTarget(injectResult.JumpBackAddress);
    const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;

    // call controller input check
    // 3 parameters (reverse push order):
    // - enum button_to_check
    //   In CS this just very cleanly lists the controller buttons, but in TX's PC port this is a
    //   bit of a mess. As far as I can tell, they hijacked the existing controller buttons' primary
    //   actions for the PC-build button remapping feature, then added a second layer of button
    //   remapping somewhere else, and the result is very jank and breaks very easily if you want
    //   custom button mapping layouts. See the function at 0x436c20 for this remapping first layer
    //   of remapping, I have no idea at the moment where the second layer happens.
    //   The values passed to the function we're calling here correspond as follows:
    //   -  0x0 /  0 = Navigation Menu
    //   -  0x1 /  1 = Confirm
    //   -  0x2 /  2 = Cancel
    //   -  0x3 /  3 = Change Partners
    //   -  0x4 /  4 = Menu Tab Left
    //   -  0x5 /  5 = Menu Tab Right
    //   -  0x6 /  6 = Switch Support
    //   -  0x7 /  7 = EX Skill
    //   -  0x8 /  8 = Map
    //   -  0x9 /  9 = Camp Menu
    //   -  0xa / 10 = (not mapped to anything)
    //   -  0xb / 11 = Reset Camera
    //   -  0xc / 12 = Menu Up
    //   -  0xd / 13 = Menu Right
    //   -  0xe / 14 = Menu Down
    //   -  0xf / 15 = Menu Left
    //   - 0x10 / 16 = Move Up
    //   - 0x11 / 17 = Move Right
    //   - 0x12 / 18 = Move Down
    //   - 0x13 / 19 = Move Left
    //   - 0x14 / 20 = Camera Up
    //   - 0x15 / 21 = Camera Right
    //   - 0x16 / 22 = Camera Down
    //   - 0x17 / 23 = Camera Left
    //   - 0x18 / 24 = Confirm (forwards to action 1)
    //   - 0x19 / 25 = Cancel (forwards to both action 2 and action 27, triggers on either)
    //   - 0x1a / 26 = hardcoded to left mouse button
    //   - 0x1b / 27 = hardcoded to right mouse button
    //   - 0x1c / 28 = hardcoded to middle mouse button
    //   - 0x1d / 29 = hardcoded to mouse wheel up
    //   - 0x1e / 30 = hardcoded to mouse wheel down
    //   - 0x1f / 31 = (not mapped to anything)
    //   Since button 10 appears to be completely unused, we'll use that for our internal Turbo key.
    //   For completeness, the actions that don't show up in this list are:
    //   Message Log, Cross Drive, Items Menu, NiAR, Jump, Attack, Talk, Skill, Lock Target,
    //   Voicemail, X-Strike Lock, Dodge, Dash, X-Strike Attack
    // - enum check_type
    //   - 0 == currently held
    //   - 1 == unconsumed new press (consumes it)
    //   - 2 == some kind of auto-repeat every x frames?
    // - bool also_check_stick_on_dpad
    WriteInstruction16(codespace, 0x6a00); // push 0
    WriteInstruction16(codespace, 0x6a00); // push 0
    WriteInstruction16(codespace, 0x6a0a); // push 10
    WriteInstruction16(codespace, 0x8b0d); // mov ecx,dword ptr[ed8appPtr]
    std::memcpy(codespace, &ed8appPtr, 4);
    codespace += 4;
    WriteInstruction16(codespace, 0x8b01);   // mov eax,dword ptr[ecx]
    WriteInstruction24(codespace, 0x8b404c); // mov eax,dword ptr[eax + 0x4c]
    WriteInstruction16(codespace, 0xffd0);   // call eax

    // call HandleTurbo()
    BranchHelper4Byte handle_turbo;
    void* handleTurboFunc = HandleTurbo;
    handle_turbo.SetTarget(static_cast<char*>(handleTurboFunc));
    WriteInstruction24(codespace, 0x8d4dfc);       // lea ecx,dword ptr[ebp - 4]
    WriteInstruction48(codespace, 0x8d96f8100000); // lea edx,dword ptr[esi + 10f8h]
    Emit_PUSH_R32(codespace, R32::EAX);
    handle_turbo.WriteJump(codespace, JumpCondition::CALL);

    // go back to code
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // finally, hook the function that translates the PC button mappings into the in-engine
    // controller buttons, and replace case 10 in the switch with code that checks the turbo button
    char* addressOfTurboCheckPush = codespace;
    WriteInstruction16(codespace, 0x6a00 | Index_TurboMode); // push Index_TurboMode
    BranchHelper4Byte go_to_common_call;
    go_to_common_call.SetTarget(readInputCommonCall);
    go_to_common_call.WriteJump(codespace, JumpCondition::JMP);
    {
        PageUnprotect page(logger, readInputCase10, 1);
        std::memcpy(readInputCase10, &addressOfTurboCheckPush, 4);
    }

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
