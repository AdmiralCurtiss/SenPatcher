#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherFixChecksumWindow : public SenTools::GUI::Window {
    SenPatcherFixChecksumWindow(GuiState& state);
    SenPatcherFixChecksumWindow(const SenPatcherFixChecksumWindow& other) = delete;
    SenPatcherFixChecksumWindow(SenPatcherFixChecksumWindow&& other) = delete;
    SenPatcherFixChecksumWindow& operator=(const SenPatcherFixChecksumWindow& other) = delete;
    SenPatcherFixChecksumWindow& operator=(SenPatcherFixChecksumWindow&& other) = delete;
    ~SenPatcherFixChecksumWindow();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Fix Save File Checksum";
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
