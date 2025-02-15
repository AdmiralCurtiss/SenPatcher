#pragma once

#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherMainWindow : public SenTools::GUI::Window {
    SenPatcherMainWindow();
    SenPatcherMainWindow(const SenPatcherMainWindow& other) = delete;
    SenPatcherMainWindow(SenPatcherMainWindow&& other) = delete;
    SenPatcherMainWindow& operator=(const SenPatcherMainWindow& other) = delete;
    SenPatcherMainWindow& operator=(SenPatcherMainWindow&& other) = delete;
    ~SenPatcherMainWindow() override;

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    bool RenderContents(GuiState& state);
    void HandlePendingWindowRequest(GuiState& state);
    bool HasPendingWindowRequest() const;

    enum class PendingWindowType {
        None,
        CS1Patch,
        CS1SystemData,
        CS2Patch,
        CS2SystemData,
        CS3Patch,
        CS4Patch,
        ReveriePatch,
        TXPatch,
    };

    FileBrowser GameBrowser;

    PendingWindowType PendingWindowRequest = PendingWindowType::None;
    std::string PendingWindowSelectedPath;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
