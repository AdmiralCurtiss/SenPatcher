#include "gui_senpatcher_cs2_system_data_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "sen2/system_data.h"
#include "util/file.h"
#include "util/result.h"
#include "util/scope.h"
#include "util/text.h"

namespace {
static constexpr const char* MAPPINGS[] = {"Zoom In",
                                           "Zoom Out",
                                           "Zoom In/Out",
                                           "Reset Camera",
                                           "Turn Character",
                                           "Change Leader (Next)",
                                           "Change Leader (Previous)",
                                           "Dash",
                                           "Walk"};
} // namespace

namespace SenTools::GUI {
struct SenPatcherCS2SystemDataWindow::WorkThreadState {
    std::string FilePath;
    SenLib::Sen2::SystemData Data;

    HyoutaUtils::Result<bool, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    WorkThreadState(std::string filePath, SenLib::Sen2::SystemData data)
      : FilePath(std::move(filePath)), Data(data), Result(false), Thread([this]() -> void {
          auto doneGuard = HyoutaUtils::MakeScopeGuard([&]() { IsDone.store(true); });
          try {
              std::string tmpPath = FilePath + ".tmp";
              HyoutaUtils::IO::File f(std::string_view(tmpPath), HyoutaUtils::IO::OpenMode::Write);
              if (!f.IsOpen()) {
                  Result = std::string("Failed to open output file.");
                  return;
              }
              auto tmpFileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { f.Delete(); });
              auto newData = Data.Serialize();
              if (f.Write(newData.data(), newData.size()) != newData.size()) {
                  Result = std::string("Failed to write output file.");
                  return;
              }
              if (!f.Rename(std::string_view(FilePath))) {
                  Result = std::string("Failed to rename output file.");
                  return;
              }
              tmpFileScope.Dispose();
              Result = std::string("Successfully wrote System Data.");
          } catch (...) {
              Result = std::string("Unexpected error.");
          }
      }) {}

    ~WorkThreadState() {
        if (Thread.joinable()) {
            Thread.join();
        }
    }
};

SenPatcherCS2SystemDataWindow::SenPatcherCS2SystemDataWindow(
    GuiState& state,
    std::string_view filePath,
    const SenLib::Sen2::SystemData& systemData)
  : FilePath(filePath), Data(systemData) {
    // TODO: Is there a better way to get imgui to handle windows where the user can create as many
    // copies as they want at will?
    sprintf(WindowID.data(), "%s##W%zx", WindowTitle, state.WindowIndexCounter++);
}

SenPatcherCS2SystemDataWindow::~SenPatcherCS2SystemDataWindow() = default;

bool SenPatcherCS2SystemDataWindow::RenderFrame(GuiState& state) {
    ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_Once);
    bool open = true;
    bool visible = ImGui::Begin(WindowID.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open;
    }

    {
        auto configScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (WorkThread) {
            ImGui::BeginDisabled();
        } else {
            configScope.Dispose();
        }

        ImGui::Text("Path: %s", FilePath.c_str());

        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(
            "In-Game Button Remapping\nThe PS3 version of this game allowed some buttons to be "
            "remapped. This feature was, for some reason, removed in the PC version's Settings "
            "menu, but the actual mapping logic still exists and can be used. Note that some of "
            "these options may not work correctly in the PC version, especially when playing with "
            "mouse and keyboard. If you have issues revert to the defaults.");
        ImGui::PopTextWrapPos();

        if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Mapping", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();

            const auto add_dropdown = [&](const char* comboLabel,
                                          std::string_view buttonName,
                                          uint16_t& mapping,
                                          int defaultMapping,
                                          bool allowTurnCharacter) -> void {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(buttonName.data(), buttonName.data() + buttonName.size());
                ImGui::TableNextColumn();
                const bool valid = (mapping < IM_ARRAYSIZE(MAPPINGS));
                const int m = static_cast<int>(mapping);
                std::string defaultLabel = std::string(MAPPINGS[defaultMapping]) + " (default)";
                if (ImGui::BeginCombo(
                        comboLabel,
                        valid ? (m == defaultMapping ? defaultLabel.c_str() : MAPPINGS[m])
                              : "(invalid value)",
                        ImGuiComboFlags_HeightLargest)) {
                    for (int n = 0; n < IM_ARRAYSIZE(MAPPINGS); ++n) {
                        if (!allowTurnCharacter && n == 4) {
                            continue;
                        }
                        const bool is_selected = (m == n);
                        if (ImGui::Selectable(
                                (n == defaultMapping ? defaultLabel.c_str() : MAPPINGS[n]),
                                is_selected)) {
                            mapping = static_cast<uint16_t>(n);
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            };
            add_dropdown("##C0", "D-Pad Up", Data.DPadUpButtonMapping, 0, true);
            add_dropdown("##C1", "D-Pad Down", Data.DPadDownButtonMapping, 1, true);
            add_dropdown("##C2", "D-Pad Left", Data.DPadLeftButtonMapping, 4, true);
            add_dropdown("##C3", "D-Pad Right", Data.DPadRightButtonMapping, 4, true);
            add_dropdown("##C4", "L1 / LB", Data.L1ButtonMapping, 5, false);
            add_dropdown("##C5", "R1 / RB", Data.R1ButtonMapping, 7, false);
            add_dropdown("##C6", "Circle / B", Data.CircleButtonMapping, 8, false);
            ImGui::EndTable();
        }

        if (ImGui::Button("Save", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
            StatusMessage = "Writing...";
            WorkThread =
                std::make_unique<SenPatcherCS2SystemDataWindow::WorkThreadState>(FilePath, Data);
        }
    }

    if (WorkThread && WorkThread->IsDone.load()) {
        WorkThread->Thread.join();
        if (WorkThread->Result.IsError()) {
            StatusMessage = WorkThread->Result.GetErrorValue();
        } else {
            StatusMessage.clear();
        }
        WorkThread.reset();
    }

    if (!StatusMessage.empty()) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
        ImGui::PopTextWrapPos();
    }

    return open;
}
} // namespace SenTools::GUI