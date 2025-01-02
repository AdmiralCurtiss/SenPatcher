#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherExtractPkaWindow : public SenTools::GUI::Window {
    SenPatcherExtractPkaWindow(GuiState& state);

    bool RenderFrame(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Extract PKA";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct ExtractionThreadState;
    std::unique_ptr<ExtractionThreadState> ExtractionThread = nullptr;
};
} // namespace SenTools::GUI
