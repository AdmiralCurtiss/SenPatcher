#include "gui_senpatcher_fix_checksum_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "sentools/common_paths.h"
#include "sentools/save_checksum_fix/save_checksum_fix.h"
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

SenPatcherFixChecksumWindow::SenPatcherFixChecksumWindow(GuiState& state) {
    // TODO: Is there a better way to get imgui to handle windows where the user can create as many
    // copies as they want at will?
    sprintf(WindowID.data(), "%s##W%zx", WindowTitle, state.WindowIndexCounter++);
}

SenPatcherFixChecksumWindow::~SenPatcherFixChecksumWindow() = default;

static void InitFileBrowser(std::string_view currentPath, FileBrowser& browser, bool forSaving) {
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
                      false);
    } else {
        browser.Reset(forSaving ? FileBrowserMode::SaveNewFile : FileBrowserMode::OpenExistingFile,
                      currentPath,
                      std::move(filters),
                      "dat",
                      forSaving,
                      false);
    }
}

bool SenPatcherFixChecksumWindow::RenderFrame(GuiState& state) {
    ImGui::SetNextWindowSize(ImVec2(400, 175), ImGuiCond_Once);
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

        if (ImGui::BeginTable("Table", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Input path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Input", InputPath.data(), InputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseInput")) {
                InitFileBrowser(
                    HyoutaUtils::TextUtils::StripToNull(InputPath), InputFileBrowser, false);
                ImGui::OpenPopup("Select file to compress");
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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
            ImGui::TextUnformatted("Output path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Output", OutputPath.data(), OutputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            if (ImGui::Button("Browse...##BrowseOutput")) {
                InitFileBrowser(
                    HyoutaUtils::TextUtils::StripToNull(OutputPath), OutputFileBrowser, true);
                ImGui::OpenPopup("Select target file");
            }

            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

        if (ImGui::Button("Fix Checksum", ImVec2(-1.0f, 0.0f)) && !WorkThread) {
            StatusMessage = "Fixing...";
            WorkThread = std::make_unique<SenPatcherFixChecksumWindow::WorkThreadState>(
                std::string(HyoutaUtils::TextUtils::StripToNull(InputPath)),
                std::string(HyoutaUtils::TextUtils::StripToNull(OutputPath)));
        }

        if (WorkThread && WorkThread->IsDone.load()) {
            WorkThread->Thread.join();
            if (WorkThread->Result.IsError()) {
                StatusMessage = ("Fixing checksum failed: " + WorkThread->Result.GetErrorValue());
            } else if (WorkThread->Result.GetSuccessValue()
                       == SaveChecksumFixResult::AlreadyCorrect) {
                StatusMessage = "Checksum already correct, no need to fix.";
            } else {
                StatusMessage = "Successfully fixed checksum.";
            }
            WorkThread.reset();
        }

        if (!StatusMessage.empty()) {
            ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(StatusMessage.data(),
                                   StatusMessage.data() + StatusMessage.size());
            ImGui::PopTextWrapPos();
        }
    }

    return open;
}
} // namespace SenTools::GUI