#include "gui_senpatcher_extract_p3a_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "p3a/unpackfs.h"
#include "sentools_imgui_utils.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools::GUI {
struct SenPatcherExtractP3AWindow::ExtractionThreadState {
    std::string InputPath;
    std::string OutputPath;
    bool GenerateJson;

    HyoutaUtils::Result<SenPatcher::UnpackP3AResult, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    ExtractionThreadState(std::string inputPath, std::string outputPath, bool generateJson)
      : InputPath(std::move(inputPath))
      , OutputPath(std::move(outputPath))
      , GenerateJson(generateJson)
      , Result(SenPatcher::UnpackP3AResult::Success)
      , Thread([this]() -> void {
          try {
              Result = SenPatcher::UnpackP3A(InputPath, OutputPath, "*", GenerateJson, false);
          } catch (...) {
              Result = std::string("Unexpected error.");
          }
          IsDone.store(true);
      }) {}

    ~ExtractionThreadState() {
        if (Thread.joinable()) {
            Thread.join();
        }
    }
};

SenPatcherExtractP3AWindow::SenPatcherExtractP3AWindow(GuiState& state) {
    // TODO: Is there a better way to get imgui to handle windows where the user can create as many
    // copies as they want at will?
    sprintf(WindowID.data(), "%s##W%zx", WindowTitle, state.WindowIndexCounter++);
}

SenPatcherExtractP3AWindow::~SenPatcherExtractP3AWindow() = default;

bool SenPatcherExtractP3AWindow::RenderFrame(GuiState& state) {
    ImGui::SetNextWindowSize(ImVec2(400, 175), ImGuiCond_Once);
    bool open = true;
    bool visible = ImGui::Begin(WindowID.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open || ExtractionThread;
    }

    {
        auto configScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (ExtractionThread) {
            ImGui::BeginDisabled();
        } else {
            configScope.Dispose();
        }

        if (ImGui::BeginTable("Table", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("P3A path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Input", InputPath.data(), InputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseInput")) {
                std::vector<FileFilter> filters;
                filters.reserve(2);
                filters.push_back(FileFilter{"P3A file (*.p3a)", "*.p3a"});
                filters.push_back(FileFilter{"All files (*.*)", "*"});
                InputFileBrowser.Reset(FileBrowserMode::OpenExistingFile,
                                       HyoutaUtils::TextUtils::StripToNull(InputPath),
                                       std::move(filters),
                                       "p3a",
                                       false,
                                       false);
                ImGui::OpenPopup("Select P3A to unpack");
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
            bool modal_open = true;
            if (ImGui::BeginPopupModal(
                    "Select P3A to unpack", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                FileBrowserResult result =
                    InputFileBrowser.RenderFrame(state, "Select P3A to unpack");
                if (result != FileBrowserResult::None) {
                    if (result == FileBrowserResult::FileSelected) {
                        HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
                            InputPath, InputFileBrowser.GetSelectedPath(), true);

                        // pre-fill output too if not yet done so
                        if (HyoutaUtils::TextUtils::StripToNull(OutputPath).empty()) {
                            HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
                                OutputPath,
                                std::string(InputFileBrowser.GetSelectedPath()) + ".ex",
                                true);
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Output path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Output", OutputPath.data(), OutputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseOutput")) {
                std::vector<FileFilter> filters;
                filters.reserve(1);
                filters.push_back(FileFilter{"Target Directory (new directory name)", ""});
                OutputFileBrowser.Reset(FileBrowserMode::SaveNewFile,
                                        HyoutaUtils::TextUtils::StripToNull(OutputPath),
                                        std::move(filters),
                                        "",
                                        false,
                                        false);
                ImGui::OpenPopup("Select target directory name (will be created)");
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
            modal_open = true;
            if (ImGui::BeginPopupModal("Select target directory name (will be created)",
                                       &modal_open,
                                       ImGuiWindowFlags_NoSavedSettings)) {
                FileBrowserResult result = OutputFileBrowser.RenderFrame(
                    state, "Select target directory name (will be created)");
                if (result != FileBrowserResult::None) {
                    if (result == FileBrowserResult::FileSelected) {
                        HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
                            OutputPath, OutputFileBrowser.GetSelectedPath(), true);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::Checkbox("Generate .json for repacking", &GenerateJson);
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }

        if (ImGuiUtils::ButtonFullWidth("Extract") && !ExtractionThread) {
            StatusMessage = "Extracting...";
            ExtractionThread = std::make_unique<SenPatcherExtractP3AWindow::ExtractionThreadState>(
                std::string(HyoutaUtils::TextUtils::StripToNull(InputPath)),
                std::string(HyoutaUtils::TextUtils::StripToNull(OutputPath)),
                GenerateJson);
        }
    }

    if (ExtractionThread && ExtractionThread->IsDone.load()) {
        ExtractionThread->Thread.join();
        if (ExtractionThread->Result.IsError()) {
            StatusMessage = ("Extraction failed: " + ExtractionThread->Result.GetErrorValue());
        } else {
            StatusMessage = "Extraction successful.";
        }
        ExtractionThread.reset();
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
        if (ExtractionThread) {
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

    return open || ExtractionThread;
}
} // namespace SenTools::GUI
