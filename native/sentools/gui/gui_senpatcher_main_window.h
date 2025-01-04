#pragma once

#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherMainWindow : public SenTools::GUI::Window {
    bool RenderFrame(GuiState& state);

private:
    void RenderContents(GuiState& state);
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
};
} // namespace SenTools::GUI
