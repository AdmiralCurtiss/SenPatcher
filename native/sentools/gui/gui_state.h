#pragma once

#include <memory>
#include <vector>

#include "imgui.h"

#include "gui_user_settings.h"
#include "window_id_management.h"

namespace SenTools::GUI {
struct Window;
}

namespace SenTools {
struct GuiState {
    // A Window may add new windows to this vector at any time, but not remove or modify any.
    std::vector<std::unique_ptr<GUI::Window>> Windows;

    // Window ID management. One instance per window type. This seems kind of convoluted but I'm not
    // sure if there's a better way to do this with imgui?
    SenTools::GUI::WindowIdTracker WindowIdsPatchCS1;
    SenTools::GUI::WindowIdTracker WindowIdsPatchCS2;
    SenTools::GUI::WindowIdTracker WindowIdsPatchCS3;
    SenTools::GUI::WindowIdTracker WindowIdsPatchCS4;
    SenTools::GUI::WindowIdTracker WindowIdsPatchReverie;
    SenTools::GUI::WindowIdTracker WindowIdsPatchTX;
    SenTools::GUI::WindowIdTracker WindowIdsSysDataCS1;
    SenTools::GUI::WindowIdTracker WindowIdsSysDataCS2;
    SenTools::GUI::WindowIdTracker WindowIdsExtractP3A;
    SenTools::GUI::WindowIdTracker WindowIdsExtractPKG;
    SenTools::GUI::WindowIdTracker WindowIdsExtractPKA;
    SenTools::GUI::WindowIdTracker WindowIdsFixChecksum;
    SenTools::GUI::WindowIdTracker WindowIdsDecompressType1;
    SenTools::GUI::WindowIdTracker WindowIdsCompressType1;

    // User settings, like recently used paths.
    SenTools::GuiUserSettings GuiSettings;

    float CurrentDpi = 0.0f;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ~GuiState();
};
} // namespace SenTools
