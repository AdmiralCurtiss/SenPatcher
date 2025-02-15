#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include "gui_file_browser.h"
#include "gui_window.h"
#include "sen1/system_data.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct SenPatcherCS1SystemDataWindow : public SenTools::GUI::Window {
    SenPatcherCS1SystemDataWindow(GuiState& state,
                                  std::string_view filePath,
                                  const SenLib::Sen1::SystemData& systemData);
    SenPatcherCS1SystemDataWindow(const SenPatcherCS1SystemDataWindow& other) = delete;
    SenPatcherCS1SystemDataWindow(SenPatcherCS1SystemDataWindow&& other) = delete;
    SenPatcherCS1SystemDataWindow& operator=(const SenPatcherCS1SystemDataWindow& other) = delete;
    SenPatcherCS1SystemDataWindow& operator=(SenPatcherCS1SystemDataWindow&& other) = delete;
    ~SenPatcherCS1SystemDataWindow();

    bool RenderFrame(GuiState& state) override;
    void Cleanup(GuiState& state) override;

private:
    static constexpr char WindowTitle[] = "Edit Trails of Cold Steel System Data";
    std::array<char, GetWindowIdBufferLength(sizeof(WindowTitle))> WindowIdString;
    size_t WindowId;

    std::string FilePath;
    SenLib::Sen1::SystemData Data;

    std::string StatusMessage;

    struct WorkThreadState;
    std::unique_ptr<WorkThreadState> WorkThread;
};
} // namespace SenTools::GUI
