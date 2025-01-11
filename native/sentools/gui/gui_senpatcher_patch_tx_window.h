#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "sentools/senpatcher_dll_loader.h"
#include "util/file.h"

namespace SenTools::GUI {
struct SenPatcherPatchTXWindow : public SenTools::GUI::Window {
    SenPatcherPatchTXWindow(GuiState& state,
                            std::string gamePath,
                            std::string patchDllPath,
                            HyoutaUtils::IO::File patchDllFile,
                            SenPatcherDllIdentificationResult patchDllInfo);
    SenPatcherPatchTXWindow(const SenPatcherPatchTXWindow& other) = delete;
    SenPatcherPatchTXWindow(SenPatcherPatchTXWindow&& other) = delete;
    SenPatcherPatchTXWindow& operator=(const SenPatcherPatchTXWindow& other) = delete;
    SenPatcherPatchTXWindow& operator=(SenPatcherPatchTXWindow&& other) = delete;
    ~SenPatcherPatchTXWindow();

    bool RenderFrame(GuiState& state) override;
    void UpdateInstalledDllInfo();

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
    std::string PatchDllPath;
    HyoutaUtils::IO::File PatchDllFile;
    SenPatcherDllIdentificationResult PatchDllInfo;
    SenPatcherDllIdentificationResult InstalledDllInfo;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
