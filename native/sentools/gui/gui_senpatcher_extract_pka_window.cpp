#include "gui_senpatcher_extract_pka_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "sentools/pka_extract/pka_extract.h"
#include "sentools_imgui_utils.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools::GUI {
struct SenPatcherExtractPkaWindow::ExtractionThreadState {
    std::string InputPath;
    std::string OutputPath;

    HyoutaUtils::Result<ExtractPkaResult, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    ExtractionThreadState(std::string inputPath, std::string outputPath)
      : InputPath(std::move(inputPath))
      , OutputPath(std::move(outputPath))
      , Result(ExtractPkaResult::Success)
      , Thread([this]() -> void {
          try {
              Result = SenTools::ExtractPka(InputPath, OutputPath, {}, false);
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

void SenPatcherExtractPkaWindow::Cleanup(GuiState& state) {
    state.WindowIdsExtractPKA.ReturnId(WindowId);
}

SenPatcherExtractPkaWindow::SenPatcherExtractPkaWindow(GuiState& state)
  : WindowId(GenerateWindowId(state.WindowIdsExtractPKA,
                              WindowIdString.data(),
                              WindowIdString.size(),
                              WindowTitle,
                              sizeof(WindowTitle))) {}

SenPatcherExtractPkaWindow::~SenPatcherExtractPkaWindow() = default;

bool SenPatcherExtractPkaWindow::RenderFrame(GuiState& state) {
    ImGuiUtils::SetInitialNextWindowSizeWidthOnly(400.0f * state.CurrentDpi);
    bool open = true;
    bool visible = ImGui::Begin(WindowIdString.data(), &open, ImGuiWindowFlags_None);
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
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("PKA path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Input", InputPath.data(), InputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseInput")) {
                std::vector<FileFilter> filters;
                filters.reserve(2);
                filters.push_back(FileFilter{"CS/Reverie PKA file (*.pka)", "*.pka"});
                filters.push_back(FileFilter{"All files (*.*)", "*"});
                InputFileBrowser.Reset(FileBrowserMode::OpenExistingFile,
                                       HyoutaUtils::TextUtils::StripToNull(InputPath),
                                       std::move(filters),
                                       "pka",
                                       false,
                                       false,
                                       SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
                ImGui::OpenPopup("Select PKA to unpack");
            }

            ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
            bool modal_open = true;
            if (ImGui::BeginPopupModal(
                    "Select PKA to unpack", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                FileBrowserResult result =
                    InputFileBrowser.RenderFrame(state, "Select PKA to unpack");
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
            ImGui::AlignTextToFramePadding();
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
                                        false,
                                        SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
                ImGui::OpenPopup("Select target directory name (will be created)");
            }

            ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
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

            ImGui::EndTable();
        }

        if (ImGuiUtils::ButtonFullWidth("Extract") && !ExtractionThread) {
            StatusMessage = "Extracting...";
            ExtractionThread = std::make_unique<SenPatcherExtractPkaWindow::ExtractionThreadState>(
                std::string(HyoutaUtils::TextUtils::StripToNull(InputPath)),
                std::string(HyoutaUtils::TextUtils::StripToNull(OutputPath)));
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
