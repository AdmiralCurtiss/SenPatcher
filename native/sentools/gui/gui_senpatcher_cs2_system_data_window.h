#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "sen2/system_data.h"

namespace SenTools::GUI {
struct SenPatcherCS2SystemDataWindow : public SenTools::GUI::Window {
    SenPatcherCS2SystemDataWindow(GuiState& state,
                                  std::string_view filePath,
                                  const SenLib::Sen2::SystemData& systemData);
    SenPatcherCS2SystemDataWindow(const SenPatcherCS2SystemDataWindow& other) = delete;
    SenPatcherCS2SystemDataWindow(SenPatcherCS2SystemDataWindow&& other) = delete;
    SenPatcherCS2SystemDataWindow& operator=(const SenPatcherCS2SystemDataWindow& other) = delete;
    SenPatcherCS2SystemDataWindow& operator=(SenPatcherCS2SystemDataWindow&& other) = delete;
    ~SenPatcherCS2SystemDataWindow();

    bool RenderFrame(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Edit Trails of Cold Steel System Data";
    // size is: actual title (including null) + "##W" + index converted to hex
    std::array<char, sizeof(WindowTitle) + 3 + sizeof(size_t) * 2> WindowID{};

    std::string FilePath;
    SenLib::Sen2::SystemData Data;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
