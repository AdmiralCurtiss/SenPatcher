#pragma once

#include "gui_senpatcher_main_window.h"

namespace SenTools {
struct GuiState {
    GUI::SenPatcherMainWindow MainWindow;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float f = 0.0f;
    int counter = 0;
};
} // namespace SenTools
