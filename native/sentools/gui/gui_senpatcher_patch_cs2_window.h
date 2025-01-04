#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherPatchCS2Window : public SenTools::GUI::Window {
    SenPatcherPatchCS2Window(GuiState& state, std::string_view gamePath);
    SenPatcherPatchCS2Window(const SenPatcherPatchCS2Window& other) = delete;
    SenPatcherPatchCS2Window(SenPatcherPatchCS2Window&& other) = delete;
    SenPatcherPatchCS2Window& operator=(const SenPatcherPatchCS2Window& other) = delete;
    SenPatcherPatchCS2Window& operator=(SenPatcherPatchCS2Window&& other) = delete;
    ~SenPatcherPatchCS2Window();

    bool RenderFrame(GuiState& state) override;

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool CheckBoxBattleAutoSkip = true;
        bool CheckBoxTurboToggle = false;
        bool CheckBoxFixVoiceFileLang = true;
        bool CheckBoxBgmEnqueueingLogic = true;
        bool CheckBoxPatchAudioThread = true;
        bool CheckBoxControllerMapping = true;
        bool CheckBoxArtsSupport = true;
        bool CheckBoxForce0Kerning = false;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxDisablePauseOnFocusLoss = false;
        bool CheckBoxForceXInput = false;
        bool CheckBoxFixBattleScopeCrash = true;
    };

private:
    static constexpr char WindowTitle[] = "Patch Trails of Cold Steel II";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    Settings GameSettings;
    std::string GamePath;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
