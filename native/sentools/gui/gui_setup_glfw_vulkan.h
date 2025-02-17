#pragma once

#include <functional>

struct ImGuiIO;
struct ImVec4;

namespace SenTools {
struct GuiState;

int RunGuiGlfwVulkan(GuiState& state,
                     const char* windowTitle,
                     const ImVec4& backgroundColor,
                     const std::function<void(ImGuiIO& io, GuiState& state)>& loadFontsCallback,
                     const std::function<bool(ImGuiIO& io, GuiState& state)>& renderFrameCallback,
                     const std::function<void(ImGuiIO& io, GuiState& state)>& loadIniCallback,
                     const std::function<void(ImGuiIO& io, GuiState& state)>& saveIniCallback);
} // namespace SenTools
