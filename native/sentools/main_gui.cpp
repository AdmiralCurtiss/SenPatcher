#include "main_gui.h"

#include <string_view>

#include "imgui.h"

#include "gui_fonts.h"
#include "gui_senpatcher_main_window.h"
#include "gui_state.h"
#include "gui_window.h"
#include "senpatcher_version.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#include "gui_setup_dx11.h"
#else
#include "gui_setup_glfw_vulkan.h"
#endif

namespace SenTools {
static void RenderFrame(ImGuiIO& io, GuiState& state) {
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

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You
    // can browse its code to learn more about Dear ImGui!).
    if (state.show_demo_window)
        ImGui::ShowDemoWindow(&state.show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a
    // named window.
    {
        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text."); // Display some text (you can use a format
                                                  // strings too)
        ImGui::Checkbox("Demo Window",
                        &state.show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &state.show_another_window);

        ImGui::SliderFloat(
            "float", &state.f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color",
                          (float*)&state.clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return
                                     // true when edited/activated)
            state.counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", state.counter);

        ImGui::Text(
            "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (state.show_another_window) {
        ImGui::Begin(
            "Another Window",
            &state
                 .show_another_window); // Pass a pointer to our bool variable (the window will have
                                        // a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            state.show_another_window = false;
        ImGui::End();
    }
}

int RunGui(int argc, char** argvUtf8) {
    GuiState state;
    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherMainWindow>());
    const std::string_view windowTitle("SenPatcher " SENPATCHER_VERSION);
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(windowTitle.data(), windowTitle.size());
    return RunGuiDX11(state, wstr ? wstr->c_str() : L"SenPatcher", LoadFonts, RenderFrame);
#else
    return RunGuiGlfwVulkan(state, windowTitle.data(), LoadFonts, RenderFrame);
#endif
}
} // namespace SenTools
