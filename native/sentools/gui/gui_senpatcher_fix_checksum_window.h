#pragma once

#include <array>
#include <memory>
#include <string>

#include "gui_file_browser.h"
#include "gui_window.h"

namespace SenTools::GUI {
struct SenPatcherFixChecksumWindow : public SenTools::GUI::Window {
    SenPatcherFixChecksumWindow(GuiState& state);
    SenPatcherFixChecksumWindow(const SenPatcherFixChecksumWindow& other) = delete;
    SenPatcherFixChecksumWindow(SenPatcherFixChecksumWindow&& other) = delete;
    SenPatcherFixChecksumWindow& operator=(const SenPatcherFixChecksumWindow& other) = delete;
    SenPatcherFixChecksumWindow& operator=(SenPatcherFixChecksumWindow&& other) = delete;
    ~SenPatcherFixChecksumWindow();

    bool RenderFrame(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Fix Save File Checksum";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    std::array<char, 1024> InputPath{};
    std::array<char, 1024> OutputPath{};
    std::string StatusMessage;

    FileBrowser InputFileBrowser;
    FileBrowser OutputFileBrowser;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI