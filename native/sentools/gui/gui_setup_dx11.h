#pragma once

#include <functional>

struct ImGuiIO;

namespace SenTools {
struct GuiState;

int RunGuiDX11(GuiState& state,
               const wchar_t* windowTitle,
               const std::function<void(ImGuiIO& io, GuiState& state)>& loadFontsCallback,
               const std::function<void(ImGuiIO& io, GuiState& state)>& renderFrameCallback);
} // namespace SenTools
