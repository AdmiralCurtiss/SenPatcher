#pragma once

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherMainWindow : public SenTools::GUI::Window {
    bool RenderFrame(GuiState& state);

private:
    FileBrowser GameBrowser;
};
} // namespace SenTools::GUI
