#include "gui_senpatcher_main_window.h"

#include <string_view>

#include "imgui.h"

#include "gui_senpatcher_extract_pka_window.h"
#include "gui_senpatcher_extract_pkg_window.h"
#include "gui_state.h"
#include "senpatcher_version.h"
#include "util/scope.h"

namespace SenTools::GUI {
static void TextUnformatted(std::string_view sv) {
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}
static void TextUnformattedRightAlign(std::string_view sv) {
    float width = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x;
    ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetCursorPosX() + width));
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}

bool SenPatcherMainWindow::RenderFrame(GuiState& state) {
    bool visible = ImGui::Begin("SenPatcher", nullptr, ImGuiWindowFlags_MenuBar);
    const auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return true;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Toolbox")) {
            if (ImGui::MenuItem("Extract PKG...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherExtractPkgWindow>(state));
            }
            if (ImGui::MenuItem("Extract PKA...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherExtractPkaWindow>(state));
            }
            if (ImGui::MenuItem("Fix Checksum of CS4 save...")) {
            }
            if (ImGui::MenuItem("Decompress PKG Type 1 (little endian)...")) {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    TextUnformattedRightAlign("SenPatcher " SENPATCHER_VERSION);

    ImGui::Spacing();

    TextUnformatted("Trails of Cold Steel: (XSEED PC release version 1.6)");
    if (ImGui::Button("Patch game##1", ImVec2(-1.0f, 0.0f))) {
    }
    if (ImGui::Button("Edit System Save Data##1", ImVec2(-1.0f, 0.0f))) {
    }

    ImGui::Spacing();

    TextUnformatted("Trails of Cold Steel II: (XSEED PC release version 1.4, 1.4.1, or 1.4.2)");
    if (ImGui::Button("Patch game##2", ImVec2(-1.0f, 0.0f))) {
    }
    if (ImGui::Button("Edit System Save Data##2", ImVec2(-1.0f, 0.0f))) {
    }

    ImGui::Spacing();

    TextUnformatted("Trails of Cold Steel III: (NISA PC release version 1.07)");
    if (ImGui::Button("Patch game##3", ImVec2(-1.0f, 0.0f))) {
    }

    ImGui::Spacing();

    TextUnformatted("Trails of Cold Steel IV: (NISA PC release version 1.2.2)");
    if (ImGui::Button("Patch game##4", ImVec2(-1.0f, 0.0f))) {
    }

    ImGui::Spacing();

    TextUnformatted("Trails into Reverie: (NISA PC release version 1.1.5)");
    if (ImGui::Button("Patch game##5", ImVec2(-1.0f, 0.0f))) {
    }

    ImGui::Spacing();

    TextUnformatted("Tokyo Xanadu eX+: (Aksys PC release version 1.08)");
    if (ImGui::Button("Patch game##X", ImVec2(-1.0f, 0.0f))) {
    }

    return true;
}
} // namespace SenTools::GUI