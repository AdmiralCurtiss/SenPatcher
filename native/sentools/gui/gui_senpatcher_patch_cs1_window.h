#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherPatchCS1Window : public SenTools::GUI::Window {
    SenPatcherPatchCS1Window(GuiState& state, std::string_view gamePath);
    SenPatcherPatchCS1Window(const SenPatcherPatchCS1Window& other) = delete;
    SenPatcherPatchCS1Window(SenPatcherPatchCS1Window&& other) = delete;
    SenPatcherPatchCS1Window& operator=(const SenPatcherPatchCS1Window& other) = delete;
    SenPatcherPatchCS1Window& operator=(SenPatcherPatchCS1Window&& other) = delete;
    ~SenPatcherPatchCS1Window();

    bool RenderFrame(GuiState& state) override;

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool CheckBoxBattleAutoSkip = true;
        bool CheckBoxTurboToggle = false;
        bool CheckBoxFixVoiceFileLang = true;
        bool CheckBoxBgmEnqueueingLogic = true;
        bool CheckBoxArtsSupport = true;
        bool CheckBoxForce0Kerning = false;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxDisablePauseOnFocusLoss = false;
        bool CheckBoxForceXInput = false;
        bool CheckBoxAllowR2InTurboMode = false;
        int ComboBoxTurboModeKey = 7;
    };

private:
    static constexpr char WindowTitle[] = "Patch Trails of Cold Steel";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    Settings GameSettings;
    std::string GamePath;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
