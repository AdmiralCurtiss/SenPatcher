#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherExtractPkaWindow : public SenTools::GUI::Window {
    SenPatcherExtractPkaWindow(GuiState& state);
    SenPatcherExtractPkaWindow(const SenPatcherExtractPkaWindow& other) = delete;
    SenPatcherExtractPkaWindow(SenPatcherExtractPkaWindow&& other) = delete;
    SenPatcherExtractPkaWindow& operator=(const SenPatcherExtractPkaWindow& other) = delete;
    SenPatcherExtractPkaWindow& operator=(SenPatcherExtractPkaWindow&& other) = delete;
    ~SenPatcherExtractPkaWindow();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Extract PKA";
    std::array<char, GetWindowIdBufferLength(sizeof(WindowTitle))> WindowIdString;
    size_t WindowId;

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct ExtractionThreadState;
    std::unique_ptr<ExtractionThreadState> ExtractionThread;
};
} // namespace SenTools::GUI
