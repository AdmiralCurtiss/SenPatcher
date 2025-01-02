#pragma once

#include <functional>

struct ImGuiIO;

namespace SenTools {
struct GuiState;

int RunGuiGlfwVulkan(GuiState& state,
                     const char* windowTitle,
                     const std::function<void(ImGuiIO& io, GuiState& state)>& loadFontsCallback,
                     const std::function<void(ImGuiIO& io, GuiState& state)>& renderFrameCallback);
} // namespace SenTools
