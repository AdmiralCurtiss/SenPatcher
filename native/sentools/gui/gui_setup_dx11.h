#pragma once

#include <functional>

struct ImGuiIO;
struct ImVec4;

namespace SenTools {
struct GuiState;

int RunGuiDX11(GuiState& state,
               const wchar_t* windowTitle,
               const ImVec4& backgroundColor,
               const std::function<void(ImGuiIO& io, GuiState& state)>& loadFontsCallback,
               const std::function<bool(ImGuiIO& io, GuiState& state)>& renderFrameCallback,
               const std::function<void(ImGuiIO& io, GuiState& state)>& loadIniCallback,
               const std::function<void(ImGuiIO& io, GuiState& state)>& saveIniCallback);
} // namespace SenTools
