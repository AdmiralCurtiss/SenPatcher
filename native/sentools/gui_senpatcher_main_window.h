#pragma once

#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherMainWindow : public SenTools::GUI::Window {
    bool RenderFrame(GuiState& state);
};
} // namespace SenTools::GUI
