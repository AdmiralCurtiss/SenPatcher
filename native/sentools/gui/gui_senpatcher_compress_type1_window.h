#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherCompressType1Window : public SenTools::GUI::Window {
    SenPatcherCompressType1Window(GuiState& state);
    SenPatcherCompressType1Window(const SenPatcherCompressType1Window& other) = delete;
    SenPatcherCompressType1Window(SenPatcherCompressType1Window&& other) = delete;
    SenPatcherCompressType1Window& operator=(const SenPatcherCompressType1Window& other) = delete;
    SenPatcherCompressType1Window& operator=(SenPatcherCompressType1Window&& other) = delete;
    ~SenPatcherCompressType1Window();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Compress Type 1";
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
