#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherPatchCS3Window : public SenTools::GUI::Window {
    SenPatcherPatchCS3Window(GuiState& state, std::string_view gamePath);
    SenPatcherPatchCS3Window(const SenPatcherPatchCS3Window& other) = delete;
    SenPatcherPatchCS3Window(SenPatcherPatchCS3Window&& other) = delete;
    SenPatcherPatchCS3Window& operator=(const SenPatcherPatchCS3Window& other) = delete;
    SenPatcherPatchCS3Window& operator=(SenPatcherPatchCS3Window&& other) = delete;
    ~SenPatcherPatchCS3Window();

    bool RenderFrame(GuiState& state) override;

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool CheckBoxTurboToggle = false;
        bool CheckBoxFixButtonRemapping = true;
        bool CheckBoxAllowNightmare = true;
        bool CheckBoxBgmEnqueueingLogic = true;
        bool CheckBoxControllerMapping = true;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxDisablePauseOnFocusLoss = false;
        bool CheckBoxForceXInput = false;
    };

private:
    static constexpr char WindowTitle[] = "Patch Trails of Cold Steel III";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    Settings GameSettings;
    std::string GamePath;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
