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

static void __fastcall HandleTurbo(float* timestep, bool buttonHeld) {
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

    if (turboActiveThisFrame) {
        *timestep = (*timestep * TurboFactor);
    }

    // the CS games clamp the timestep, presumably to avoid physics going haywire on too long
    // timesteps. so let's do that too, with the same value
    if (!(*timestep < 0.0666666666f)) {
        *timestep = 0.0666666666f;
    }
    return;
}

void PatchTurboMode(PatchExecData& execData, bool makeToggle, float factor) {
    if (factor >= 1.0f || factor <= 6.0f) {
        TurboFactor = factor;
    } else {
        TurboFactor = 2.0f;
    }

    TurboIsToggle = makeToggle;

    HyoutaUtils::Logger& logger = *execData.Logger;
    char* textRegion = execData.TextRegion;
    GameVersion version = execData.Version;
    char* codespace = execData.Codespace;
    using namespace SenPatcher::x86;

    // TODO: full list of steam addresses

    char* const entryPoint = GetCodeAddressSteamGog(version, textRegion, 0x437521, 0x435d41);
    char* const ed8appPtr = GetCodeAddressSteamGog(version, textRegion, 0xb4d888, 0xb4c888);
    char* const readInputCase10 = GetCodeAddressSteamGog(version, textRegion, 0, 0x436e8c);
    char* const readInputCommonCall = GetCodeAddressSteamGog(version, textRegion, 0, 0x436c4f);
    char* const countButtonMappingsToAlloc =
        GetCodeAddressSteamGog(version, textRegion, 0x4058d2, 0x405412) + 1;
    char* const countButtonMappingsToInit =
        GetCodeAddressSteamGog(version, textRegion, 0x405940, 0x405480) + 2;
    char* const countButtonMappingsToWriteToIni =
        GetCodeAddressSteamGog(version, textRegion, 0, 0x406ed3) + 2;
    char* const countButtonMappingsToReadFromIni =
        GetCodeAddressSteamGog(version, textRegion, 0, 0x406d8f) + 2;

    // TODO: are all of these needed?
    char* const countButtonMappingsInRemappingGUI1 =
        GetCodeAddressSteamGog(version, textRegion, 0, 0x67d7b4) + 2;
    char* const countButtonMappingsInRemappingGUI2 =
        GetCodeAddressSteamGog(version, textRegion, 0, 0x6739a3) + 2;
    int* const countButtonMappingsInRemappingGUI3 =
        (int*)(GetCodeAddressSteamGog(version, textRegion, 0, 0x6a8be6) + 2);
    int* const countButtonMappingsInRemappingGUI4 =
        (int*)(GetCodeAddressSteamGog(version, textRegion, 0, 0x673c81) + 2);


    // TODO: add a string to display for the turbo button label
    // TODO: the scroll bar in the remapping menu still assumes the old row count

    // increase the amount of button mappings by one so we can have our own remappable turbo button
    {
        PageUnprotect page(logger, countButtonMappingsToAlloc, 1);
        (*countButtonMappingsToAlloc) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToInit, 1);
        (*countButtonMappingsToInit) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToWriteToIni, 1);
        (*countButtonMappingsToWriteToIni) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsToReadFromIni, 1);
        (*countButtonMappingsToReadFromIni) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI1, 1);
        (*countButtonMappingsInRemappingGUI1) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI2, 1);
        (*countButtonMappingsInRemappingGUI2) += 1;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI3, 4);
        (*countButtonMappingsInRemappingGUI3) += 4;
    }
    {
        PageUnprotect page(logger, countButtonMappingsInRemappingGUI4, 4);
        (*countButtonMappingsInRemappingGUI4) += 4;
    }

    // then, hook the turbo mode function
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
    WriteInstruction24(codespace, 0x8d4dfc); // lea ecx,dword ptr[ebp - 4]
    Emit_MOV_R32_R32(codespace, R32::EDX, R32::EAX);
    handle_turbo.WriteJump(codespace, JumpCondition::CALL);

    // go back to code
    std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
    codespace += overwrittenInstructions.size();
    jump_back.WriteJump(codespace, JumpCondition::JMP);

    // finally, hook the function that translates the PC button mappings into the in-engine
    // controller buttons, and replace case 10 in the switch with code that checks the turbo button
    char* addressOfTurboCheckPush = codespace;
    WriteInstruction16(codespace, 0x6a25); // push 0x25
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
