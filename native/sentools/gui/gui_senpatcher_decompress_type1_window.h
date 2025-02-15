#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherDecompressType1Window : public SenTools::GUI::Window {
    SenPatcherDecompressType1Window(GuiState& state);
    SenPatcherDecompressType1Window(const SenPatcherDecompressType1Window& other) = delete;
    SenPatcherDecompressType1Window(SenPatcherDecompressType1Window&& other) = delete;
    SenPatcherDecompressType1Window&
        operator=(const SenPatcherDecompressType1Window& other) = delete;
    SenPatcherDecompressType1Window& operator=(SenPatcherDecompressType1Window&& other) = delete;
    ~SenPatcherDecompressType1Window();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Decompress Type 1";
    std::array<char, GetWindowIdBufferLength(sizeof(WindowTitle))> WindowIdString;
    size_t WindowId;

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
