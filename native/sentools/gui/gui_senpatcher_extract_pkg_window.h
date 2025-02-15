#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherExtractPkgWindow : public SenTools::GUI::Window {
    SenPatcherExtractPkgWindow(GuiState& state);
    SenPatcherExtractPkgWindow(const SenPatcherExtractPkgWindow& other) = delete;
    SenPatcherExtractPkgWindow(SenPatcherExtractPkgWindow&& other) = delete;
    SenPatcherExtractPkgWindow& operator=(const SenPatcherExtractPkgWindow& other) = delete;
    SenPatcherExtractPkgWindow& operator=(SenPatcherExtractPkgWindow&& other) = delete;
    ~SenPatcherExtractPkgWindow();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Extract PKG";
    std::array<char, GetWindowIdBufferLength(sizeof(WindowTitle))> WindowIdString;
    size_t WindowId;

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    bool GenerateJson = false;
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct ExtractionThreadState;
    std::unique_ptr<ExtractionThreadState> ExtractionThread;
};
} // namespace SenTools::GUI
