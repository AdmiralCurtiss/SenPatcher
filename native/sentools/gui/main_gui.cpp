#include "main_gui.h"

#include <string_view>

#include "imgui.h"

#include "gui_fonts.h"
#include "gui_senpatcher_main_window.h"
#include "gui_state.h"
#include "gui_user_settings.h"
#include "gui_window.h"
#include "senpatcher_version.h"
#include "sentools/common_paths.h"
#include "util/file.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#include "gui_setup_dx11.h"
#else
#include "gui_setup_glfw_vulkan.h"
#endif

namespace SenTools {
static bool RenderFrame(ImGuiIO& io, GuiState& state) {
    size_t windowCount = state.Windows.size();
    for (size_t i = 0; i < windowCount;) {
        GUI::Window* window = state.Windows[i].get();
        ImGui::PushID(static_cast<const void*>(window));
        if (!window->RenderFrame(state)) {
            state.Windows.erase(state.Windows.begin() + i);
            --windowCount;
        } else {
            ++i;
        }
        ImGui::PopID();
    }

    // ImGui::ShowDemoWindow();

    return windowCount > 0;
}

int RunGui(int argc, char** argvUtf8) {
    GuiState state;
    std::optional<std::string> guiSettingsFolder =
        CommonPaths::GetLocalSenPatcherGuiSettingsFolder();
    std::string userIniPath;
    if (guiSettingsFolder) {
        userIniPath = *guiSettingsFolder;
        userIniPath.append("/gui.ini");
        if (!SenTools::LoadUserSettingsFromIni(state.GuiSettings, userIniPath)) {
            SenTools::LoadUserSettingsFromCSharpUserConfig(state.GuiSettings, *guiSettingsFolder);
        }
    }
    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherMainWindow>());
    const std::string_view windowTitle("SenPatcher " SENPATCHER_VERSION);
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(windowTitle.data(), windowTitle.size());
    int rv = RunGuiDX11(state, wstr ? wstr->c_str() : L"SenPatcher", LoadFonts, RenderFrame);
#else
    int rv = RunGuiGlfwVulkan(state, windowTitle.data(), LoadFonts, RenderFrame);
#endif
    if (guiSettingsFolder) {
        HyoutaUtils::IO::CreateDirectory(std::string_view(*guiSettingsFolder));
        SenTools::WriteUserSettingsToIni(state.GuiSettings, userIniPath);
    }
    return rv;
}
} // namespace SenTools
