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
struct SenPatcherPatchCS4Window : public SenTools::GUI::Window {
    SenPatcherPatchCS4Window(GuiState& state,
                             std::string gamePath,
                             std::string patchDllPath,
                             HyoutaUtils::IO::File patchDllFile,
                             SenPatcherDllIdentificationResult patchDllInfo);
    SenPatcherPatchCS4Window(const SenPatcherPatchCS4Window& other) = delete;
    SenPatcherPatchCS4Window(SenPatcherPatchCS4Window&& other) = delete;
    SenPatcherPatchCS4Window& operator=(const SenPatcherPatchCS4Window& other) = delete;
    SenPatcherPatchCS4Window& operator=(SenPatcherPatchCS4Window&& other) = delete;
    ~SenPatcherPatchCS4Window();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;
    void UpdateInstalledDllInfo();

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool CheckBoxAllowNightmare = true;
        bool CheckBoxBgmEnqueueingLogic = true;
        bool CheckBoxButtonLayout = false;
        bool ComboBoxForceJp = true;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxDisablePauseOnFocusLoss = false;
    };

private:
    static constexpr char WindowTitle[] = "Patch Trails of Cold Steel IV";
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
