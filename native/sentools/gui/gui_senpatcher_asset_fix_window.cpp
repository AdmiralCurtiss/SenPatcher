#include "gui_senpatcher_asset_fix_window.h"

#include <string>

#include "imgui.h"

#include "gui_state.h"
#include "sentools_imgui_utils.h"

namespace SenTools::GUI {
void HandleAssetFixDetailButton(SenTools::GuiState& state, const std::string& details) {
    if (ImGui::Button("Show details (may contain spoilers)")) {
        ImGui::OpenPopup("Asset fixes and improvements");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    bool modal_open = true;
    if (ImGui::BeginPopupModal(
            "Asset fixes and improvements", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
        static constexpr char closeLabel[] = "Close";
        auto textSize = ImGui::CalcTextSize(closeLabel, nullptr, true);
        float framePaddingWidth = ImGui::GetStyle().FramePadding.x * 2.0f;
        float framePaddingHeight =
            ImGui::GetStyle().FramePadding.y * 2.0f + ImGui::GetStyle().ItemSpacing.y;

        if (ImGui::BeginChild("AssetFixDetails",
                              ImVec2(0.0f, -(textSize.y + framePaddingHeight)),
                              ImGuiChildFlags_Borders)) {
            ImGui::TextUnformatted(details.data(), details.data() + details.size());
        }
        ImGui::EndChild();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                             - (textSize.x + framePaddingWidth));
        if (ImGui::Button(closeLabel)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
} // namespace SenTools::GUI
