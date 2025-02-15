#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherExtractP3AWindow : public SenTools::GUI::Window {
    SenPatcherExtractP3AWindow(GuiState& state);
    SenPatcherExtractP3AWindow(const SenPatcherExtractP3AWindow& other) = delete;
    SenPatcherExtractP3AWindow(SenPatcherExtractP3AWindow&& other) = delete;
    SenPatcherExtractP3AWindow& operator=(const SenPatcherExtractP3AWindow& other) = delete;
    SenPatcherExtractP3AWindow& operator=(SenPatcherExtractP3AWindow&& other) = delete;
    ~SenPatcherExtractP3AWindow();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Extract P3A";
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
