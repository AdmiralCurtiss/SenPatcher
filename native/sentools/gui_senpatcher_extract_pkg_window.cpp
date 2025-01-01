#include "gui_senpatcher_extract_pkg_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
#include "pkg_extract.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools::GUI {
struct SenPatcherExtractPkgWindow::ExtractionThreadState {
    std::string InputPath;
    std::string OutputPath;
    bool GenerateJson;

    HyoutaUtils::Result<ExtractPkgResult, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    ExtractionThreadState(std::string inputPath, std::string outputPath, bool generateJson)
      : InputPath(std::move(inputPath))
      , OutputPath(std::move(outputPath))
      , GenerateJson(generateJson)
      , Result(ExtractPkgResult::Success)
      , Thread([this]() -> void {
          try {
              Result = SenTools::ExtractPkg(InputPath, OutputPath, GenerateJson);
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

SenPatcherExtractPkgWindow::SenPatcherExtractPkgWindow(GuiState& state) {
    // TODO: Is there a better way to get imgui to handle windows where the user can create as many
    // copies as they want at will?
    sprintf(WindowID.data(), "%s##W%zx", WindowTitle, state.WindowIndexCounter++);
}

bool SenPatcherExtractPkgWindow::RenderFrame(GuiState& state) {
    ImGui::SetNextWindowSize(ImVec2(400, 175), ImGuiCond_Once);
    bool open = true;
    bool visible = ImGui::Begin(WindowID.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open;
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
            ImGui::TextUnformatted("PKG path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Input", InputPath.data(), InputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            ImGui::Button("Browse...##BrowseInput");

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Output path:");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(
                "##Output", OutputPath.data(), OutputPath.size(), ImGuiInputTextFlags_ElideLeft);
            ImGui::TableNextColumn();
            ImGui::Button("Browse...##BrowseOutput");

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::Checkbox("Generate .json for repacking", &GenerateJson);
            ImGui::TableNextColumn();

            ImGui::EndTable();
        }

        if (ImGui::Button("Extract", ImVec2(-1.0f, 0.0f)) && !ExtractionThread) {
            StatusMessage = "Extracting...";
            ExtractionThread = std::make_unique<SenPatcherExtractPkgWindow::ExtractionThreadState>(
                std::string(HyoutaUtils::TextUtils::StripToNull(InputPath)),
                std::string(HyoutaUtils::TextUtils::StripToNull(OutputPath)),
                GenerateJson);
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

        if (!StatusMessage.empty()) {
            ImGui::TextUnformatted(StatusMessage.data(),
                                   StatusMessage.data() + StatusMessage.size());
        }
    }

    return open;
}
} // namespace SenTools::GUI
