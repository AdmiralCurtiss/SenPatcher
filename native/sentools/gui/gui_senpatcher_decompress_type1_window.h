#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherDecompressType1Window : public SenTools::GUI::Window {
    SenPatcherDecompressType1Window(GuiState& state);

    bool RenderFrame(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Decompress Type 1";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread = nullptr;
};
} // namespace SenTools::GUI
