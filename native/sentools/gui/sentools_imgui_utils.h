#pragma once

#include <string_view>

#include "imgui.h"

#include "util/scope.h"

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

inline void SetInitialNextWindowSizeWidthOnly(float width,
                                              ImGuiCond cond = ImGuiCond_FirstUseEver) {
    float windowWidth = ImGui::GetMainViewport()->WorkSize.x;
    float popupWidth = width;
    if (popupWidth > windowWidth) {
        popupWidth = windowWidth;
    }
    ImGui::SetNextWindowSize(ImVec2(popupWidth, 0.0f), cond);
}

inline void GamepadNavigableHelperTooltip(const char* id,
                                          std::string_view outerText,
                                          std::string_view innerText,
                                          float tooltipWidth,
                                          bool requiresHighBox = false) {
    auto& style = ImGui::GetStyle();
    ImVec2 textSize = ImGui::CalcTextSize(outerText.data(), outerText.data() + outerText.size());
    if (requiresHighBox) {
        textSize.y += style.FramePadding.y * 2.0f;
    }
    ImGui::InvisibleButton(id, textSize, ImGuiButtonFlags_EnableNav);
    const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone);
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - textSize.x);
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::TextUnformatted(outerText.data(), outerText.data() + outerText.size());
    ImGui::PopStyleColor();
    if (hovered && ImGui::BeginTooltip()) {
        ImGui::PushTextWrapPos(tooltipWidth);
        ImGui::TextUnformatted(innerText.data(), innerText.data() + innerText.size());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

inline auto ConditionallyDisabledScope(bool disabled) {
    if (disabled) {
        ImGui::BeginDisabled();
    }
    return HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); }, !disabled);
}
} // namespace ImGuiUtils
