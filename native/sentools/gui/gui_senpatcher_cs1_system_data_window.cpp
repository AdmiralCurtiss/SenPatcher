#include "gui_senpatcher_cs1_system_data_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "sen1/system_data.h"
#include "sentools_imgui_utils.h"
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
                                           "Change Leader (Previous)"};
} // namespace

namespace SenTools::GUI {
struct SenPatcherCS1SystemDataWindow::WorkThreadState {
    std::string FilePath;
    SenLib::Sen1::SystemData Data;

    HyoutaUtils::Result<bool, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    WorkThreadState(std::string filePath, SenLib::Sen1::SystemData data)
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
              if (!f.Flush()) {
                  Result = std::string("Failed to flush output file.");
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

void SenPatcherCS1SystemDataWindow::Cleanup(GuiState& state) {
    state.WindowIdsSysDataCS1.ReturnId(WindowId);
}

SenPatcherCS1SystemDataWindow::SenPatcherCS1SystemDataWindow(
    GuiState& state,
    std::string_view filePath,
    const SenLib::Sen1::SystemData& systemData)
  : WindowId(GenerateWindowId(state.WindowIdsSysDataCS1,
                              WindowIdString.data(),
                              WindowIdString.size(),
                              WindowTitle,
                              sizeof(WindowTitle)))
  , FilePath(filePath)
  , Data(systemData) {}

SenPatcherCS1SystemDataWindow::~SenPatcherCS1SystemDataWindow() = default;

bool SenPatcherCS1SystemDataWindow::RenderFrame(GuiState& state) {
    ImGuiUtils::SetInitialNextWindowSizeWidthOnly(400.0f * state.CurrentDpi);
    bool open = true;
    bool visible = ImGui::Begin(WindowIdString.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open || WorkThread;
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
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(buttonName.data(), buttonName.data() + buttonName.size());
                ImGui::TableNextColumn();
                const bool valid = (mapping < IM_ARRAYSIZE(MAPPINGS));
                const int m = static_cast<int>(mapping);
                std::string defaultLabel = std::string(MAPPINGS[defaultMapping]) + " (default)";
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
            ImGui::EndTable();
        }

        if (ImGuiUtils::ButtonFullWidth("Save", 40.0f * state.CurrentDpi) && !WorkThread) {
            StatusMessage = "Writing...";
            WorkThread =
                std::make_unique<SenPatcherCS1SystemDataWindow::WorkThreadState>(FilePath, Data);
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

    static constexpr char closeLabel[] = "Close";
    float closeTextWidth = ImGui::CalcTextSize(closeLabel, nullptr, true).x;
    float closeButtonWidth = closeTextWidth + (ImGui::GetStyle().FramePadding.x * 2.0f);
    if (!StatusMessage.empty()) {
        float wrap = ImGui::GetContentRegionAvail().x - closeButtonWidth;
        ImGui::PushTextWrapPos(wrap <= FLT_MIN ? FLT_MIN : wrap);
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
        ImGui::PopTextWrapPos();
        ImGui::SameLine();
    }

    {
        auto scope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (WorkThread) {
            ImGui::BeginDisabled();
        } else {
            scope.Dispose();
        }
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                             - closeButtonWidth);
        if (ImGui::Button(closeLabel)) {
            open = false;
        }
    }

    return open || WorkThread;
}
} // namespace SenTools::GUI
