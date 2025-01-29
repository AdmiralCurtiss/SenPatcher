#pragma once

#include <string_view>

#include "imgui.h"

namespace ImGuiUtils {
inline void TextUnformatted(std::string_view sv) {
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}

inline void TextUnformattedRightAlign(std::string_view sv) {
    float width = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width);
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}
} // namespace ImGuiUtils
