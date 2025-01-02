#pragma once

#include <memory>
#include <vector>

#include "imgui.h"

namespace SenTools::GUI {
struct Window;
}

namespace SenTools {
struct GuiState {
    // A Window may add new windows to this vector at any time, but not remove or modify any.
    std::vector<std::unique_ptr<GUI::Window>> Windows;

    // Counter for global index for each Window pushed into the Windows.
    size_t WindowIndexCounter = 0;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float f = 0.0f;
    int counter = 0;

    ~GuiState();
};
} // namespace SenTools
