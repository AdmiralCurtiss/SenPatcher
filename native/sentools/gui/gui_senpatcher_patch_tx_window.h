#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherPatchTXWindow : public SenTools::GUI::Window {
    SenPatcherPatchTXWindow(GuiState& state, std::string_view gamePath);
    SenPatcherPatchTXWindow(const SenPatcherPatchTXWindow& other) = delete;
    SenPatcherPatchTXWindow(SenPatcherPatchTXWindow&& other) = delete;
    SenPatcherPatchTXWindow& operator=(const SenPatcherPatchTXWindow& other) = delete;
    SenPatcherPatchTXWindow& operator=(SenPatcherPatchTXWindow&& other) = delete;
    ~SenPatcherPatchTXWindow();

    bool RenderFrame(GuiState& state) override;

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool ComboBoxGameLanguageEnglish = true;
        bool CheckBoxSkipLogos = true;
        bool CheckBoxSkipMovies = false;
        bool CheckBoxTurboToggle = false;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxEnableBackgroundControllerInput = false;
        bool CheckBoxDisableCamAutoCenter = false;
        bool CheckBoxFixBgmResume = true;
        float NumericUpDownTurboFactor = 2.0f;
    };

private:
    static constexpr char WindowTitle[] = "Patch Tokyo Xanadu eX+";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    Settings GameSettings;
    std::string GamePath;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
