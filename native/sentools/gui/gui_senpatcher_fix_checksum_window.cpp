#include "gui_senpatcher_fix_checksum_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "sentools/common_paths.h"
#include "sentools/save_checksum_fix/save_checksum_fix.h"
#include "sentools_imgui_utils.h"
#include "util/result.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools::GUI {
struct SenPatcherFixChecksumWindow::WorkThreadState {
    std::string InputPath;
    std::string OutputPath;

    HyoutaUtils::Result<SaveChecksumFixResult, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    WorkThreadState(std::string inputPath, std::string outputPath)
      : InputPath(std::move(inputPath))
      , OutputPath(std::move(outputPath))
      , Result(SaveChecksumFixResult::Success)
      , Thread([this]() -> void {
          try {
              Result = SenTools::SaveChecksumFix(InputPath, OutputPath);
          } catch (...) {
              Result = std::string("Unexpected error.");
          }
          IsDone.store(true);
      }) {}

    ~WorkThreadState() {
        if (Thread.joinable()) {
            Thread.join();
        }
    }
};

void SenPatcherFixChecksumWindow::Cleanup(GuiState& state) {
    state.WindowIdsFixChecksum.ReturnId(WindowId);
}

SenPatcherFixChecksumWindow::SenPatcherFixChecksumWindow(GuiState& state)
  : WindowId(GenerateWindowId(state.WindowIdsFixChecksum,
                              WindowIdString.data(),
                              WindowIdString.size(),
                              WindowTitle,
                              sizeof(WindowTitle))) {}

SenPatcherFixChecksumWindow::~SenPatcherFixChecksumWindow() = default;

static void InitFileBrowser(std::string_view currentPath,
                            FileBrowser& browser,
                            bool forSaving,
                            bool useCustomFileBrowser) {
    std::vector<FileFilter> filters;
    filters.reserve(2);
    filters.push_back(FileFilter{"Save files (*.dat)", "*.dat"});
    filters.push_back(FileFilter{"All files (*.*)", "*"});

    if (currentPath.empty()) {
        auto savedGames = SenTools::CommonPaths::GetSavedGamesFolder();
        std::string saveFolder;
        if (savedGames) {
            saveFolder = (*savedGames + "/FALCOM");
        }
        browser.Reset(forSaving ? FileBrowserMode::SaveNewFile : FileBrowserMode::OpenExistingFile,
                      saveFolder,
                      "",
                      std::move(filters),
                      "dat",
                      forSaving,
                      false,
                      useCustomFileBrowser);
    } else {
        browser.Reset(forSaving ? FileBrowserMode::SaveNewFile : FileBrowserMode::OpenExistingFile,
                      currentPath,
                      std::move(filters),
                      "dat",
                      forSaving,
                      false,
                      useCustomFileBrowser);
    }
}

bool SenPatcherFixChecksumWindow::RenderFrame(GuiState& state) {
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

        if (ImGui::BeginTable("Table", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Input path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Input", InputPath.data(), InputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseInput")) {
                InitFileBrowser(HyoutaUtils::TextUtils::StripToNull(InputPath),
                                InputFileBrowser,
                                false,
                                SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
                ImGui::OpenPopup("Select file to compress");
            }

            ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
            bool modal_open = true;
            if (ImGui::BeginPopupModal(
                    "Select file to compress", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                FileBrowserResult result =
                    InputFileBrowser.RenderFrame(state, "Select file to compress");
                if (result != FileBrowserResult::None) {
                    if (result == FileBrowserResult::FileSelected) {
                        HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
                            InputPath, InputFileBrowser.GetSelectedPath(), true);

                        // pre-fill output too if not yet done so
                        if (HyoutaUtils::TextUtils::StripToNull(OutputPath).empty()) {
                            HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
                                OutputPath, std::string(InputFileBrowser.GetSelectedPath()), true);
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
                InitFileBrowser(HyoutaUtils::TextUtils::StripToNull(OutputPath),
                                OutputFileBrowser,
                                true,
                                SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
                ImGui::OpenPopup("Select target file");
            }

            ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
            modal_open = true;
            if (ImGui::BeginPopupModal(
                    "Select target file", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                FileBrowserResult result =
                    OutputFileBrowser.RenderFrame(state, "Select target file");
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

        if (ImGuiUtils::ButtonFullWidth("Fix Checksum") && !WorkThread) {
            StatusMessage = "Fixing...";
            WorkThread = std::make_unique<SenPatcherFixChecksumWindow::WorkThreadState>(
                std::string(HyoutaUtils::TextUtils::StripToNull(InputPath)),
                std::string(HyoutaUtils::TextUtils::StripToNull(OutputPath)));
        }
    }

    if (WorkThread && WorkThread->IsDone.load()) {
        WorkThread->Thread.join();
        if (WorkThread->Result.IsError()) {
            StatusMessage = ("Fixing checksum failed: " + WorkThread->Result.GetErrorValue());
        } else if (WorkThread->Result.GetSuccessValue() == SaveChecksumFixResult::AlreadyCorrect) {
            StatusMessage = "Checksum already correct, no need to fix.";
        } else {
            StatusMessage = "Successfully fixed checksum.";
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
