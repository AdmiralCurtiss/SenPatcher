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
struct SenPatcherPatchReverieWindow : public SenTools::GUI::Window {
    SenPatcherPatchReverieWindow(GuiState& state,
                                 std::string gamePath,
                                 std::string patchDllPath,
                                 HyoutaUtils::IO::File patchDllFile,
                                 SenPatcherDllIdentificationResult patchDllInfo);
    SenPatcherPatchReverieWindow(const SenPatcherPatchReverieWindow& other) = delete;
    SenPatcherPatchReverieWindow(SenPatcherPatchReverieWindow&& other) = delete;
    SenPatcherPatchReverieWindow& operator=(const SenPatcherPatchReverieWindow& other) = delete;
    SenPatcherPatchReverieWindow& operator=(SenPatcherPatchReverieWindow&& other) = delete;
    ~SenPatcherPatchReverieWindow();

    bool RenderFrame(GuiState& state) override;
    void UpdateInstalledDllInfo();

    struct Settings {
        bool CheckBoxAssetPatches = true;
        bool CheckBoxBgmEnqueueingLogic = true;
        bool CheckBoxDisableMouseCam = false;
        bool CheckBoxShowMouseCursor = false;
        bool CheckBoxDisableFpsLimitOnFocusLoss = false;
    };

private:
    static constexpr char WindowTitle[] = "Patch Trails into Reverie";
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
