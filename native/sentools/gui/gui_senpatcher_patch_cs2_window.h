#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "sentools/senpatcher_dll_loader.h"
#include "util/file.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherPatchCS2Window : public SenTools::GUI::Window {
    SenPatcherPatchCS2Window(GuiState& state,
                             std::string gamePath,
                             std::string patchDllPath,
                             HyoutaUtils::IO::File patchDllFile,
                             SenPatcherDllIdentificationResult patchDllInfo);
    SenPatcherPatchCS2Window(const SenPatcherPatchCS2Window& other) = delete;
    SenPatcherPatchCS2Window(SenPatcherPatchCS2Window&& other) = delete;
    SenPatcherPatchCS2Window& operator=(const SenPatcherPatchCS2Window& other) = delete;
    SenPatcherPatchCS2Window& operator=(SenPatcherPatchCS2Window&& other) = delete;
    ~SenPatcherPatchCS2Window();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;
    void UpdateInstalledDllInfo();

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
    std::array<char, GetWindowIdBufferLength(sizeof(WindowTitle))> WindowIdString;
    size_t WindowId;

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
