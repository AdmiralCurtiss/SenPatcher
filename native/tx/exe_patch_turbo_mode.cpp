#include "exe_patch.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "x86/emitter.h"
#include "x86/inject_jump_into.h"
#include "x86/page_unprotect.h"

// #define TX_DEBUG_NEW_BUTTONS
#ifdef TX_DEBUG_NEW_BUTTONS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace SenLib::TX {
static bool TurboActive = false;
static bool TurboIsToggle = false;
static bool TurboButtonPressedLastFrame = false;
static float TurboFactor = 0.0f;
static float RealTimeStep = 0.0f;
static float TempStoreMul = 0.0f;

using PCheckPcButtonMapping = int8_t(__cdecl*)(uint32_t button);

static PCheckPcButtonMapping s_CheckPcButtonMapping = nullptr;

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
static const char EN_ButtonName_SysAction1[] = "System 1 (Pause/Skip)";
static const char JP_ButtonName_SysAction1[] = "System 1 (Pause/Skip)";
static const char EN_ButtonName_SysAction2[] = "System 2 (Reset/Details)";
static const char JP_ButtonName_SysAction2[] = "System 2 (Reset/Details)";
static const char EN_ButtonName_ToggleAutoAdvance[] = "Toggle Auto-Advance";
static const char JP_ButtonName_ToggleAutoAdvance[] = "Toggle Auto-Advance";
static const char EN_ButtonName_ToggleFastForward[] = "Toggle Fast Forward";
static const char JP_ButtonName_ToggleFastForward[] = "Toggle Fast Forward";
static const char EN_ButtonName_ZoomIn[] = "Zoom In";
static const char JP_ButtonName_ZoomIn[] = "\xE6\x8B\xA1\xE5\xA4\xA7";
static const char EN_ButtonName_ZoomOut[] = "Zoom Out";
static const char JP_ButtonName_ZoomOut[] = "\xE7\xB8\xAE\xE5\xB0\x8F";
static const char EN_ButtonName_OpenGate[] = "Open Gate";
static const char JP_ButtonName_OpenGate[] = "Open Gate";
static const char EN_ButtonName_XStrikeLeft[] = "X-Strike Chain (Left)";
static const char JP_ButtonName_XStrikeLeft[] = "X-Strike Chain (Left)";
static const char EN_ButtonName_XStrikeRight[] = "X-Strike Chain (Right)";
static const char JP_ButtonName_XStrikeRight[] = "X-Strike Chain (Right)";
static const char EN_ButtonName_TurboMode[] = "Turbo Mode";
static const char JP_ButtonName_TurboMode[] =
    "\xE3\x82\xBF\xE3\x83\xBC\xE3\x83\x9C\xE3\x83\xA2\xE3\x83\xBC\xE3\x83\x89";
static const char EN_ButtonName_SwimmingLeft[] = "Swimming - Left";
static const char JP_ButtonName_SwimmingLeft[] = "Swimming - Left";
static const char EN_ButtonName_SwimmingRight[] = "Swimming - Right";
static const char JP_ButtonName_SwimmingRight[] = "Swimming - Right";
static const char EN_ButtonName_FishingUp[] = "Fishing - Up";
static const char JP_ButtonName_FishingUp[] = "Fishing\xE3\x83\xBB\xE4\xB8\x8A";
static const char EN_ButtonName_FishingDown[] = "Fishing - Down";
static const char JP_ButtonName_FishingDown[] = "Fishing\xE3\x83\xBB\xE4\xB8\x8B";
static const char EN_ButtonName_FishingLeft[] = "Fishing - Left";
static const char JP_ButtonName_FishingLeft[] = "Fishing\xE3\x83\xBB\xE5\xB7\xA6";
static const char EN_ButtonName_FishingRight[] = "Fishing - Right";
static const char JP_ButtonName_FishingRight[] = "Fishing\xE3\x83\xBB\xE5\x8F\xB3";
static const char EN_ButtonName_SkateboardAccel[] = "Skateboard - Accelerate";
static const char JP_ButtonName_SkateboardAccel[] = "Skateboard - Accelerate";
static const char EN_ButtonName_SkateboardJump[] = "Skateboard - Jump";
static const char JP_ButtonName_SkateboardJump[] = "Skateboard - Jump";
static const char EN_ButtonName_SkateboardBrake[] = "Skateboard - Brake";
static const char JP_ButtonName_SkateboardBrake[] = "Skateboard - Brake";

static constexpr uint32_t OldNumberOfButtons = 0x25;
static constexpr uint32_t Index_MenuAction1 = OldNumberOfButtons + 0;
static constexpr uint32_t Index_MenuAction2 = OldNumberOfButtons + 1;
static constexpr uint32_t Index_MenuZoomIn = OldNumberOfButtons + 2;
static constexpr uint32_t Index_MenuZoomOut = OldNumberOfButtons + 3;
static constexpr uint32_t Index_SysAction1 = OldNumberOfButtons + 4;
static constexpr uint32_t Index_SysAction2 = OldNumberOfButtons + 5;
static constexpr uint32_t Index_ToggleAutoAdvance = OldNumberOfButtons + 6;
static constexpr uint32_t Index_ToggleFastForward = OldNumberOfButtons + 7;
static constexpr uint32_t Index_ZoomIn = OldNumberOfButtons + 8;
static constexpr uint32_t Index_ZoomOut = OldNumberOfButtons + 9;
static constexpr uint32_t Index_OpenGate = OldNumberOfButtons + 10;
static constexpr uint32_t Index_XStrikeLeft = OldNumberOfButtons + 11;
static constexpr uint32_t Index_XStrikeRight = OldNumberOfButtons + 12;
static constexpr uint32_t Index_TurboMode = OldNumberOfButtons + 13;
static constexpr uint32_t Index_SwimmingLeft = OldNumberOfButtons + 14;
static constexpr uint32_t Index_SwimmingRight = OldNumberOfButtons + 15;
static constexpr uint32_t Index_FishingUp = OldNumberOfButtons + 16;
static constexpr uint32_t Index_FishingDown = OldNumberOfButtons + 17;
static constexpr uint32_t Index_FishingLeft = OldNumberOfButtons + 18;
static constexpr uint32_t Index_FishingRight = OldNumberOfButtons + 19;
static constexpr uint32_t Index_SkateboardAccel = OldNumberOfButtons + 20;
static constexpr uint32_t Index_SkateboardJump = OldNumberOfButtons + 21;
static constexpr uint32_t Index_SkateboardBrake = OldNumberOfButtons + 22;
static constexpr uint32_t ButtonsToAdd = 23;

// if this is triggered we need to remap the lookup array for button names
static_assert(ButtonsToAdd <= OldNumberOfButtons);

#ifdef TX_DEBUG_NEW_BUTTONS
static void __fastcall DebugFunc2(int* stack) {
    uint32_t return_address = (uint32_t)stack[1];
    const int button_to_check = stack[2];
    const int check_type = stack[3];
    const int also_check_stick = stack[4] & 0xff;

    if (button_to_check == (int)Index_TurboMode) {
        return;
    }

    if (return_address == 0x0048d137 || return_address == 0x0048e537) {
        return_address = (uint32_t)stack[6];
    }

    char buffer[200];
    sprintf(buffer,
            " 0x%08x -> CheckButton(button = %d, type = %d, also_stick = %d)\n",
            return_address,
            button_to_check,
            check_type,
            also_check_stick);
    OutputDebugStringA(buffer);

    return;
}

static void __fastcall DebugFunc3(int* stack) {
    uint32_t return_address = (uint32_t)stack[1];
    const int button_to_check = stack[2];

    char buffer[200];
    sprintf(buffer, " 0x%08x -> GetIcon(icon = %d)\n", return_address, button_to_check);
    OutputDebugStringA(buffer);

    return;
}
#endif

namespace {
struct ButtonStateData {
    uint8_t Pressed = 0; // 0 == not pressed, 1 == pressed and unconsumed, 2 == pressed and consumed
};
} // namespace

static ButtonStateData s_NewButtonStates[ButtonsToAdd];

static void __fastcall UpdateNewButtons() {
    auto& states = s_NewButtonStates;
    auto func = s_CheckPcButtonMapping;
    for (uint32_t i = 0; i < ButtonsToAdd; ++i) {
        if (func(i + OldNumberOfButtons) != 0) {
            // if it's consumed but still held keep it consumed
            if (states[i].Pressed == 0) {
                states[i].Pressed = 1;
            }
        } else {
            states[i].Pressed = 0;
        }
    }
}

static int32_t __fastcall CheckNewButtons(uint32_t button_to_check, uint32_t check_type) {
    const uint32_t button = button_to_check - OldNumberOfButtons;
    if (button >= ButtonsToAdd) {
        return 0;
    }

    auto& state = s_NewButtonStates[button];
    if (check_type == 0) {
        return (state.Pressed != 0) ? 1 : 0;
    }
    if (check_type == 1) {
        if (state.Pressed == 1) {
            state.Pressed = 2;
            return 1;
        }
        return 0;
    }

    // TODO: There's also a check_type 2 for periodic repeat, not sure if we need that?

    return 0;
}

namespace {
struct PcButtonPromptOverride {
    // the icon index this override triggers on
    int32_t IconIndex = -1;

    // the button it maps to. if multiple are set (must be in order), it switches between them over
    // time; this is used for the prompts that switch between keyboard and mouse prompt and the
    // stick prompts that switch over the four directions
    int32_t ButtonMappingIndex1 = -1;
    int32_t ButtonMappingIndex2 = -1;
    int32_t ButtonMappingIndex3 = -1;
    int32_t ButtonMappingIndex4 = -1;
};
static_assert(sizeof(PcButtonPromptOverride) == 20);
} // namespace

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

    // TODO: Add default bindings when launching without ini or resetting to default.
    // TODO: Incorrect prompts I'm aware of:
    // - We have two clashing L1/R1 prompts that attempt to use the same mappings, one of them needs
    //   to be remapped somehow. (Zoom In/Out and Menu Tab Left/Right)
    //   -> Can be seen in most menus with tabs, as well as the Message Log.
    // - The 'Open Gate' prompt has the same problem, it needs a bespoke prompt.
    // - I haven't checked the S-Craft followups but I'm sure they have the same issue.
    // - Fishing, Swimming, and Skateboarding minigames are all still wrong.
    // - I'm sure there are a million errors in tutorial messages now...

    s_CheckPcButtonMapping = reinterpret_cast<PCheckPcButtonMapping>(
        GetCodeAddressSteamGog(version, textRegion, 0x438070, 0x4368a0));
    char* const addressUpdateButtons =
        GetCodeAddressSteamGog(version, textRegion, 0x438eba, 0x4376ea);
    char* const addressCheckButton =
        GetCodeAddressSteamGog(version, textRegion, 0x4386ca, 0x436efa);

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
    char* const openMessageLogInDialogueAddress =
        GetCodeAddressSteamGog(version, textRegion, 0x6525e8, 0x650af8);

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

    char* const addressButtonMappingIconLookupArray1 =
        GetCodeAddressSteamGog(version, textRegion, 0x580601, 0x57eba1) + 2;
    char* const addressButtonMappingIconLookupLength1 =
        GetCodeAddressSteamGog(version, textRegion, 0x580610, 0x57ebb0) + 1;
    char* const addressButtonMappingIconLookupArray2a =
        GetCodeAddressSteamGog(version, textRegion, 0x581990, 0x57ff30) + 2;
    char* const addressButtonMappingIconLookupArray2b =
        GetCodeAddressSteamGog(version, textRegion, 0x58199c, 0x57ff3c) + 2;
    char* const addressButtonMappingIconLookupArray2c =
        GetCodeAddressSteamGog(version, textRegion, 0x5819ac, 0x57ff4c) + 2;
    char* const addressButtonMappingIconLookupArray2d =
        GetCodeAddressSteamGog(version, textRegion, 0x5819be, 0x57ff5e) + 2;
    char* const addressButtonMappingIconLookupArray2e =
        GetCodeAddressSteamGog(version, textRegion, 0x581a13, 0x57ffb3) + 3;
    char* const addressButtonMappingIconLookupArray2f =
        GetCodeAddressSteamGog(version, textRegion, 0x581a1c, 0x57ffbc) + 2;
    char* const addressButtonMappingIconLookupLength2 =
        GetCodeAddressSteamGog(version, textRegion, 0x581abf, 0x58005f) + 2;

    // Add new buttons to list of button prompts.
    // The implementation here is quite convoluted. In order to make the PC button prompts work, the
    // game has hijacked the function that's responsible for determining the texture and x/y
    // location to draw for any given icon ID and manually checks the IDs that need replacement to
    // point them to the correct icon in the PC prompt texture.
    // We're gonna extend that mapping array with our buttons and use a few unused IDs for them.
    {
        char* codespace = execData.Codespace;

        // copy into codespace
        PcButtonPromptOverride* originalLocationOfOverrides;
        std::memcpy(&originalLocationOfOverrides, addressButtonMappingIconLookupArray1, 4);
        static constexpr size_t originalOverrideArrayLength = 35;
        static constexpr size_t originalOverrideArrayByteLength =
            originalOverrideArrayLength * sizeof(PcButtonPromptOverride);
        std::memcpy(codespace, originalLocationOfOverrides, originalOverrideArrayByteLength);
        PcButtonPromptOverride* newLocationOfOverrides =
            reinterpret_cast<PcButtonPromptOverride*>(codespace);
        codespace += originalOverrideArrayByteLength;

        // add space for more mappings
        static constexpr size_t numberOfPromptsToAdd = 2;
        PcButtonPromptOverride dummyPrompt;
        for (size_t i = 0; i < numberOfPromptsToAdd; ++i) {
            std::memcpy(codespace, &dummyPrompt, sizeof(PcButtonPromptOverride));
            codespace += sizeof(PcButtonPromptOverride);
        }

        // update some existing mappings to point to more useful mappings
        newLocationOfOverrides[3].ButtonMappingIndex1 = Index_MenuAction1;  // #162I
        newLocationOfOverrides[4].ButtonMappingIndex1 = Index_MenuAction2;  // #163I
        newLocationOfOverrides[6].ButtonMappingIndex1 = Index_SysAction1;   // #203I
        newLocationOfOverrides[8].ButtonMappingIndex1 = Index_SysAction2;   // #110I
        newLocationOfOverrides[11].ButtonMappingIndex1 = Index_ZoomOut;     // #164I
        newLocationOfOverrides[12].ButtonMappingIndex1 = Index_ZoomIn;      // #166I
        newLocationOfOverrides[13].ButtonMappingIndex1 = Index_MenuZoomOut; // #180I
        newLocationOfOverrides[14].ButtonMappingIndex1 = Index_MenuZoomIn;  // #182I

        // add new icon -> prompt mappings
        newLocationOfOverrides[35].IconIndex = 165;
        newLocationOfOverrides[35].ButtonMappingIndex1 = 0x23; // menu tab left
        newLocationOfOverrides[36].IconIndex = 167;
        newLocationOfOverrides[36].ButtonMappingIndex1 = 0x24; // menu tab right

        // update references to array
        const auto update_ref = [&](char* const addr) -> void {
            PageUnprotect page(logger, addr, 4);
            uint32_t oldLoc = std::bit_cast<uint32_t>(originalLocationOfOverrides);
            uint32_t tmp;
            std::memcpy(&tmp, addr, 4);
            uint32_t offset = tmp - oldLoc;
            uint32_t newLoc = std::bit_cast<uint32_t>(newLocationOfOverrides) + offset;
            std::memcpy(addr, &newLoc, 4);
        };
        update_ref(addressButtonMappingIconLookupArray1);
        update_ref(addressButtonMappingIconLookupArray2a);
        update_ref(addressButtonMappingIconLookupArray2b);
        update_ref(addressButtonMappingIconLookupArray2c);
        update_ref(addressButtonMappingIconLookupArray2d);
        update_ref(addressButtonMappingIconLookupArray2e);
        update_ref(addressButtonMappingIconLookupArray2f);
        uint32_t newByteLength = originalOverrideArrayByteLength
                                 + (numberOfPromptsToAdd * sizeof(PcButtonPromptOverride));
        {
            PageUnprotect page(logger, addressButtonMappingIconLookupLength1, 4);
            std::memcpy(addressButtonMappingIconLookupLength1, &newByteLength, 4);
        }
        {
            PageUnprotect page(logger, addressButtonMappingIconLookupLength2, 4);
            std::memcpy(addressButtonMappingIconLookupLength2, &newByteLength, 4);
        }

        execData.Codespace = codespace;
    }


    // change buttons that are being checked
    const auto change_button = [&](char* address, uint32_t button) -> void {
        if (address == nullptr || address == (char*)1) {
            return;
        }
        PageUnprotect page(logger, address, 1);
        *address = static_cast<char>(button);
    };
    const auto ga = [&](uint32_t addressSteam, uint32_t addressGog) -> char* {
        if (version == GameVersion::Steam && addressSteam == 0) {
            return nullptr;
        }
        return GetCodeAddressSteamGog(version, textRegion, addressSteam, addressGog);
    };
    // clang-format off
    change_button(ga(0x49aa44, 0x499624) + 1, Index_MenuAction2);       // camp menu (on character), change equipment
    change_button(ga(0x497a87, 0x496667) + 1, Index_MenuAction1);       // item menu, discard
    change_button(ga(0x49936e, 0x497f4e) + 1, Index_MenuAction1);       // status menu, check soul level
    change_button(ga(0x499509, 0x4980e9) + 1, Index_MenuAction1);       // status menu, check skill info
    change_button(ga(0x5a27d6, 0x5a0cb6) + 1, Index_MenuAction2);       // quick travel menu, hide icons
    change_button(ga(0x64ea9e, 0x64cf6e) + 1, Index_MenuAction1);       // eclipse start menu, greed info
    change_button(ga(0x64ed1c, 0x64d1ec) + 1, Index_MenuAction2);       // eclipse start menu, change equipment
    change_button(ga(0x64e5a8, 0x64ca78) + 1, Index_MenuAction2);       // eclipse start menu (on character), change equipment
    change_button(ga(0x64ebeb, 0x64d0bb) + 1, Index_SysAction1);        // eclipse start menu, open system menu
    change_button(ga(0x64e980, 0x64ce50) + 1, Index_SysAction2);        // eclipse start menu, detailed results
    change_button(ga(0x48bbe2, 0x48a7c2) + 1, Index_MenuZoomIn);        // equip menu, zoom in
    change_button(ga(0x48bb9e, 0x48a77e) + 1, Index_MenuZoomOut);       // equip menu, zoom out
    change_button(ga(0x495c15, 0x4947f5) + 1, Index_SysAction1);        // orbment menu, auto-equip
    change_button(ga(0x495c7c, 0x49485c) + 1, Index_SysAction2);        // orbment menu, remove all
    change_button(ga(0x51c9b0, 0x51b0d0) + 1, Index_OpenGate);          // open hidden eclipse gate
    change_button(ga(0x5cac7f, 0x5c911f) + 1, Index_FishingLeft);       // fishing, left button
    change_button(ga(0x5cac9d, 0x5c913d) + 1, Index_FishingDown);       // fishing, bottom button
    change_button(ga(0x5cacbb, 0x5c915b) + 1, Index_FishingRight);      // fishing, right button
    change_button(ga(0x5cacd9, 0x5c9179) + 1, Index_FishingUp);         // fishing, top button
    change_button(ga(0x56b944, 0x569f84) + 1, Index_SwimmingLeft);      // swimming, left button
    change_button(ga(0x56b95f, 0x569f9f) + 1, Index_SwimmingRight);     // swimming, right button
    change_button(ga(0x5af9d5, 0x5ade75) + 1, Index_SysAction1);        // skateboarding, pause
    change_button(ga(0x986e3a, 0x985d9a),     Index_SkateboardAccel);   // skateboarding, accelerate
    change_button(ga(0x986e3b, 0x985d9b),     Index_SkateboardJump);    // skateboarding, jump
    change_button(ga(0x986e3c, 0x985d9c),     Index_MenuAction1);       // skateboarding, ????, no idea what this is
    change_button(ga(0x986e3d, 0x985d9d),     Index_SkateboardBrake);   // skateboarding, brake
    change_button(ga(0x986e3e, 0x985d9e),     Index_SkateboardBrake);   // skateboarding, reverse
    change_button(ga(0x4a7dbb, 0x4a693b) + 1, Index_MenuAction1);       // character viewer, perform action
    change_button(ga(0x4a7cbf, 0x4a683f) + 1, Index_MenuAction2);       // character viewer, take screenshot
    change_button(ga(0x4a7b82, 0x4a6702) + 1, Index_SysAction1);        // character viewer, open/close menu
    change_button(ga(0x4a32dc, 0x4a1e5c) + 1, Index_ZoomIn);            // character viewer, zoom in
    change_button(ga(0x4a3294, 0x4a1e14) + 1, Index_ZoomOut);           // character viewer, zoom out
    change_button(ga(0x45813b, 0x45698b) + 1, Index_MenuAction1);       // boss battle menu, switch category
    change_button(ga(0x499ebf, 0x498a9f) + 1, Index_SysAction1);        // button remapping menu (camp), restore defaults
    change_button(ga(0x65da69, 0x65bf79) + 1, Index_SysAction1);        // button remapping menu (title), restore defaults
    change_button(ga(0x573e6a, 0x57247a) + 1, Index_SysAction1);        // skip scene
    change_button(ga(0x56e74d, 0x56cd8d) + 1, Index_SysAction1);        // skip X-Strike
    change_button(ga(0x601e4b, 0x6002cb) + 1, Index_SysAction1);        // skip FMV (Start)
    change_button(ga(0x601e1d, 0x60029d) + 1, Index_SysAction2);        // skip FMV (Select)
    change_button(ga(0x601e06, 0x600286) + 1, Index_MenuAction1);       // skip FMV (Triangle)
    change_button(ga(0x601dbd, 0x60023d) + 1, Index_MenuAction2);       // skip FMV (Square)
    change_button(ga(0x573ddb, 0x5723eb) + 1, Index_ToggleFastForward); // enable fast-forward scene
    change_button(ga(0x573ccc, 0x5722dc) + 1, Index_MenuAction1);       // disable fast-forward scene (Triangle)
    change_button(ga(0x573c89, 0x572299) + 1, Index_MenuAction2);       // disable fast-forward scene (Square)
    change_button(ga(0x573ce3, 0x5722f3) + 1, Index_ToggleFastForward); // disable fast-forward scene (L1)
    change_button(ga(0x573cfa, 0x57230a) + 1, Index_ToggleAutoAdvance); // disable fast-forward scene (R1)
    change_button(ga(0x5699b4, 0x567ff4) + 1, Index_XStrikeLeft);       // X-Strike Followup (Left Only)
    change_button(ga(0x5699c1, 0x568001) + 1, Index_XStrikeRight);      // X-Strike Followup (Right Only)
    change_button(ga(0x569982, 0x567fc2) + 1, Index_XStrikeLeft);       // X-Strike Followup (Left Both)
    change_button(ga(0x569999, 0x567fd9) + 1, Index_XStrikeRight);      // X-Strike Followup (Right Both)
    change_button(ga(0x5e4b3a, 0x5e2fda) + 1, Index_ZoomIn);            // minimap, zoom in
    change_button(ga(0x5e4b86, 0x5e3026) + 1, Index_ZoomOut);           // minimap, zoom out
    change_button(ga(0x5753c9, 0x573999) + 1, Index_ToggleAutoAdvance); // textbox, toggle auto-advance
    change_button(ga(0x5e4bce, 0x5e306e) + 1, Index_SysAction1);        // minimap, escape dungeon
    change_button(ga(0x641795, 0x63fc65) + 1, Index_MenuAction1);       // regular shop, change item display
    change_button(ga(0x644a5d, 0x642f2d) + 1, Index_MenuAction1);       // trade shop, change item display
    change_button(ga(0x643a78, 0x641f48) + 1, Index_MenuAction1);       // upgrade grid/skills, change item display
    // clang-format on


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

    // fix the message log so it actually obeys the configured mapping while textboxes are open.
    // by default it uses 'menu up'...
    {
        char* codespace = execData.Codespace;

        // hook the function that translates the PC button mappings into the in-engine controller
        // buttons, and replace case 10 in the switch with code that checks the Message Log button
        char* addressOfPush = codespace;
        WriteInstruction16(codespace, 0x6a00); // push 0 (index of Message Log mapping)
        BranchHelper4Byte go_to_common_call;
        go_to_common_call.SetTarget(readInputCommonCall);
        go_to_common_call.WriteJump(codespace, JumpCondition::JMP);
        {
            PageUnprotect page(logger, readInputCase10, 1);
            std::memcpy(readInputCase10, &addressOfPush, 4);
        }

        execData.Codespace = codespace;

        // replace the code that does this weird two-step indirection to check for the message log
        // button to just call the standard button check function
        {
            char* tmp = openMessageLogInDialogueAddress;
            PageUnprotect page(logger, tmp, 13);
            WriteInstruction16(tmp, 0x6a00);   // push 0
            WriteInstruction16(tmp, 0x6a01);   // push 1
            WriteInstruction16(tmp, 0x6a0a);   // push 10
            WriteInstruction16(tmp, 0x8b01);   // mov eax,dword ptr[ecx]
            WriteInstruction24(tmp, 0x8b404c); // mov eax,dword ptr[eax + 0x4c]
            WriteInstruction16(tmp, 0xffd0);   // call eax
        }
    }

    // hook the button update function and update the state of our new buttons at the end
    {
        char* codespace = execData.Codespace;
        const auto inject = InjectJumpIntoCode<5>(logger, addressUpdateButtons, codespace);

        BranchHelper4Byte updateButtons;
        void* updateNewButtonsFunc = UpdateNewButtons;
        updateButtons.SetTarget(static_cast<char*>(updateNewButtonsFunc));
        updateButtons.WriteJump(codespace, JC::CALL);

        std::memcpy(codespace,
                    inject.OverwrittenInstructions.data(),
                    inject.OverwrittenInstructions.size());
        codespace += inject.OverwrittenInstructions.size();

        BranchHelper4Byte jmpBack;
        jmpBack.SetTarget(inject.JumpBackAddress);
        jmpBack.WriteJump(codespace, JC::JMP);
        execData.Codespace = codespace;
    }

    // hook the button check function to handle the new buttons
    {
        char* codespace = execData.Codespace;
        auto injectResult = InjectJumpIntoCode<5>(logger, addressCheckButton, codespace);


#ifdef TX_DEBUG_NEW_BUTTONS
        Emit_PUSH_R32(codespace, R32::EAX);
        Emit_PUSH_R32(codespace, R32::EBX);
        Emit_PUSH_R32(codespace, R32::ECX);
        Emit_PUSH_R32(codespace, R32::EDX);
        Emit_PUSH_R32(codespace, R32::ESI);
        Emit_PUSH_R32(codespace, R32::EDI);
        Emit_PUSH_R32(codespace, R32::EBP);

        BranchHelper4Byte call_dbg;
        void* debug_func_ptr = DebugFunc2;
        call_dbg.SetTarget(static_cast<char*>(debug_func_ptr));
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::EBP);
        call_dbg.WriteJump(codespace, JumpCondition::CALL);

        Emit_POP_R32(codespace, R32::EBP);
        Emit_POP_R32(codespace, R32::EDI);
        Emit_POP_R32(codespace, R32::ESI);
        Emit_POP_R32(codespace, R32::EDX);
        Emit_POP_R32(codespace, R32::ECX);
        Emit_POP_R32(codespace, R32::EBX);
        Emit_POP_R32(codespace, R32::EAX);
#endif


        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), 3); // cmp edi,0x1f
        codespace += 3;

        BranchHelper4Byte jump_back_not_new_button;
        jump_back_not_new_button.SetTarget(injectResult.JumpBackAddress);
        jump_back_not_new_button.WriteJump(codespace, JC::JNGE);

        BranchHelper4Byte checkNewButtons;
        void* checkNewButtonsFunc = CheckNewButtons;
        checkNewButtons.SetTarget(static_cast<char*>(checkNewButtonsFunc));

        // call our new button check function
        Emit_MOV_R32_PtrR32PlusOffset8(codespace, R32::EDX, R32::EBP, 0xc);
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::EDI);
        checkNewButtons.WriteJump(codespace, JumpCondition::CALL);

        // return the result directly
        Emit_POP_R32(codespace, R32::EDI);
        Emit_POP_R32(codespace, R32::ESI);
        Emit_POP_R32(codespace, R32::EBP);
        Emit_RET_IMM16(codespace, 0xc);

        execData.Codespace = codespace;
    }

#ifdef TX_DEBUG_NEW_BUTTONS
    {
        char* codespace = execData.Codespace;
        char* const getIconFunctionAddress =
            GetCodeAddressSteamGog(version, textRegion, 0x5817a1, 0x57fd41);
        auto injectResult = InjectJumpIntoCode<5>(logger, getIconFunctionAddress, codespace);

        const auto& overwrittenInstructions = injectResult.OverwrittenInstructions;
        std::memcpy(codespace, overwrittenInstructions.data(), overwrittenInstructions.size());
        codespace += overwrittenInstructions.size();

        Emit_PUSH_R32(codespace, R32::EAX);
        Emit_PUSH_R32(codespace, R32::EBX);
        Emit_PUSH_R32(codespace, R32::ECX);
        Emit_PUSH_R32(codespace, R32::EDX);
        Emit_PUSH_R32(codespace, R32::ESI);
        Emit_PUSH_R32(codespace, R32::EDI);
        Emit_PUSH_R32(codespace, R32::EBP);

        BranchHelper4Byte call_dbg;
        void* debug_func_ptr = DebugFunc3;
        call_dbg.SetTarget(static_cast<char*>(debug_func_ptr));
        Emit_MOV_R32_R32(codespace, R32::ECX, R32::EBP);
        call_dbg.WriteJump(codespace, JumpCondition::CALL);

        Emit_POP_R32(codespace, R32::EBP);
        Emit_POP_R32(codespace, R32::EDI);
        Emit_POP_R32(codespace, R32::ESI);
        Emit_POP_R32(codespace, R32::EDX);
        Emit_POP_R32(codespace, R32::ECX);
        Emit_POP_R32(codespace, R32::EBX);
        Emit_POP_R32(codespace, R32::EAX);

        BranchHelper4Byte jump_back;
        jump_back.SetTarget(injectResult.JumpBackAddress);
        jump_back.WriteJump(codespace, JC::JMP);

        execData.Codespace = codespace;
    }
#endif

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
        static constexpr uint32_t actionNameArrayLength = OldNumberOfButtons;
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
            *(enArrayLocation + Index_SysAction1) = JP_ButtonName_SysAction1;
            *(enArrayLocation + Index_SysAction2) = JP_ButtonName_SysAction2;
            *(enArrayLocation + Index_ToggleAutoAdvance) = JP_ButtonName_ToggleAutoAdvance;
            *(enArrayLocation + Index_ToggleFastForward) = JP_ButtonName_ToggleFastForward;
            *(enArrayLocation + Index_ZoomIn) = JP_ButtonName_ZoomIn;
            *(enArrayLocation + Index_ZoomOut) = JP_ButtonName_ZoomOut;
            *(enArrayLocation + Index_OpenGate) = JP_ButtonName_OpenGate;
            *(enArrayLocation + Index_XStrikeLeft) = JP_ButtonName_XStrikeLeft;
            *(enArrayLocation + Index_XStrikeRight) = JP_ButtonName_XStrikeRight;
            *(enArrayLocation + Index_TurboMode) = JP_ButtonName_TurboMode;
            *(enArrayLocation + Index_SwimmingLeft) = JP_ButtonName_SwimmingLeft;
            *(enArrayLocation + Index_SwimmingRight) = JP_ButtonName_SwimmingRight;
            *(enArrayLocation + Index_FishingUp) = JP_ButtonName_FishingUp;
            *(enArrayLocation + Index_FishingDown) = JP_ButtonName_FishingDown;
            *(enArrayLocation + Index_FishingLeft) = JP_ButtonName_FishingLeft;
            *(enArrayLocation + Index_FishingRight) = JP_ButtonName_FishingRight;
            *(enArrayLocation + Index_SkateboardAccel) = JP_ButtonName_SkateboardAccel;
            *(enArrayLocation + Index_SkateboardJump) = JP_ButtonName_SkateboardJump;
            *(enArrayLocation + Index_SkateboardBrake) = JP_ButtonName_SkateboardBrake;
        } else {
            *(enArrayLocation + Index_MenuAction1) = EN_ButtonName_MenuAction1;
            *(enArrayLocation + Index_MenuAction2) = EN_ButtonName_MenuAction2;
            *(enArrayLocation + Index_MenuZoomIn) = EN_ButtonName_MenuZoomIn;
            *(enArrayLocation + Index_MenuZoomOut) = EN_ButtonName_MenuZoomOut;
            *(enArrayLocation + Index_SysAction1) = EN_ButtonName_SysAction1;
            *(enArrayLocation + Index_SysAction2) = EN_ButtonName_SysAction2;
            *(enArrayLocation + Index_ToggleAutoAdvance) = EN_ButtonName_ToggleAutoAdvance;
            *(enArrayLocation + Index_ToggleFastForward) = EN_ButtonName_ToggleFastForward;
            *(enArrayLocation + Index_ZoomIn) = EN_ButtonName_ZoomIn;
            *(enArrayLocation + Index_ZoomOut) = EN_ButtonName_ZoomOut;
            *(enArrayLocation + Index_OpenGate) = EN_ButtonName_OpenGate;
            *(enArrayLocation + Index_XStrikeLeft) = EN_ButtonName_XStrikeLeft;
            *(enArrayLocation + Index_XStrikeRight) = EN_ButtonName_XStrikeRight;
            *(enArrayLocation + Index_TurboMode) = EN_ButtonName_TurboMode;
            *(enArrayLocation + Index_SwimmingLeft) = EN_ButtonName_SwimmingLeft;
            *(enArrayLocation + Index_SwimmingRight) = EN_ButtonName_SwimmingRight;
            *(enArrayLocation + Index_FishingUp) = EN_ButtonName_FishingUp;
            *(enArrayLocation + Index_FishingDown) = EN_ButtonName_FishingDown;
            *(enArrayLocation + Index_FishingLeft) = EN_ButtonName_FishingLeft;
            *(enArrayLocation + Index_FishingRight) = EN_ButtonName_FishingRight;
            *(enArrayLocation + Index_SkateboardAccel) = EN_ButtonName_SkateboardAccel;
            *(enArrayLocation + Index_SkateboardJump) = EN_ButtonName_SkateboardJump;
            *(enArrayLocation + Index_SkateboardBrake) = EN_ButtonName_SkateboardBrake;
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
    //   For completeness, the actions that don't show up in this list are:
    //   Message Log, Cross Drive, Items Menu, NiAR, Jump, Attack, Talk, Skill, Lock Target,
    //   Voicemail, X-Strike Lock, Dodge, Dash, X-Strike Attack
    //   Note that button 0xa/10 is mapped to Open Message Log in SenPatcher!
    // - enum check_type
    //   - 0 == currently held
    //   - 1 == unconsumed new press (consumes it)
    //   - 2 == some kind of auto-repeat every x frames?
    // - bool also_check_stick_on_dpad
    WriteInstruction16(codespace, 0x6a00);                   // push 0
    WriteInstruction16(codespace, 0x6a00);                   // push 0
    WriteInstruction16(codespace, 0x6a00 | Index_TurboMode); // push Index_TurboMode
    WriteInstruction16(codespace, 0x8b0d);                   // mov ecx,dword ptr[ed8appPtr]
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

    execData.Codespace = codespace;
}
} // namespace SenLib::TX
