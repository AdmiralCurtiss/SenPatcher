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

inline bool ButtonFullWidth(const char* label, float height = 0.0f) {
    return ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, height));
}

inline bool ButtonRightAlign(const char* label) {
    float textWidth = ImGui::CalcTextSize(label, nullptr, true).x;
    float framePadding = ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                         - (textWidth + framePadding));
    return ImGui::Button(label);
}

inline void SetNextWindowSizeForStandardPopup(ImGuiCond cond = ImGuiCond_Appearing) {
    float windowWidth = ImGui::GetMainViewport()->WorkSize.x;
    float popupWidth = windowWidth * 0.3f;
    if (popupWidth < 320.0f) {
        popupWidth = 320.0f;
    }
    if (popupWidth > windowWidth) {
        popupWidth = windowWidth;
    }
    ImGui::SetNextWindowSize(ImVec2(popupWidth, 0.0f), cond);
}

inline void SetNextWindowSizeForNearFullscreenPopup(ImGuiCond cond = ImGuiCond_Appearing) {
    auto windowSize = ImGui::GetMainViewport()->WorkSize;
    ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.9f, windowSize.y * 0.9f), cond);
}
} // namespace ImGuiUtils
