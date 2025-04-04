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

// #define HOOK_MAGIC_DESCRIPTION_GENERATOR
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
#include <memory>

#include "sen5/tbl.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef CreateDirectory
#endif

namespace SenTools {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
uint32_t s_SelectedMagicIndex;
void* s_memory;
std::unique_ptr<SenLib::Sen5::Tbl> s_LoadedTbl;
std::string s_SelectedMagicFlags;
std::string s_SelectedMagicName;
std::string s_SelectedMagicDesc;
#endif

static bool RenderFrame(ImGuiIO& io, GuiState& state) {
    size_t windowCount = state.Windows.size();
    for (size_t i = 0; i < windowCount;) {
        GUI::Window* window = state.Windows[i].get();
        ImGui::PushID(static_cast<const void*>(window));
        if (!window->RenderFrame(state)) {
            window->Cleanup(state);
            state.Windows.erase(state.Windows.begin() + i);
            --windowCount;
        } else {
            ++i;
        }
        ImGui::PopID();
    }

    // ImGui::ShowDemoWindow();

#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
    if (s_memory != nullptr) {
        bool visible = ImGui::Begin("SenHook", nullptr, ImGuiWindowFlags_None);
        if (visible) {
            uint32_t* trigger = std::bit_cast<uint32_t*>(s_memory);
            bool b = (*trigger != 0);
            ImGui::Checkbox("Enable Override", &b);
            *trigger = b ? 1 : 0;
            const float drag_speed = 0.2f;
            uint32_t value = s_SelectedMagicIndex;
            if (ImGui::Button("-")) {
                --value;
            }
            ImGui::SameLine();
            ImGui::DragScalar(
                "##ValueOverride", ImGuiDataType_U32, &value, drag_speed, NULL, NULL, "%u");
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                ++value;
            }

            if (value != s_SelectedMagicIndex) {
                s_SelectedMagicIndex = value;
                s_SelectedMagicFlags.clear();
                s_SelectedMagicName.clear();
                s_SelectedMagicDesc.clear();
                if (s_LoadedTbl) {
                    if (value < s_LoadedTbl->Entries.size()) {
                        auto& e = s_LoadedTbl->Entries[value];
                        if (e.Name == "magic") {
                            SenLib::Sen5::MagicData m(e.Data.data(), e.Data.size());
                            value = m.idx;
                            s_SelectedMagicFlags = std::move(m.flags);
                            s_SelectedMagicName = std::move(m.name);
                            s_SelectedMagicDesc = std::move(m.desc);
                        }
                    }
                }
                *(trigger + 1) = value;
            }

            ImGui::Text("Current ID: %u", *(trigger + 1));
            ImGui::Text("%s", s_SelectedMagicFlags.c_str());
            ImGui::Text("%s", s_SelectedMagicName.c_str());
            ImGui::Text("%s", s_SelectedMagicDesc.c_str());
        }
        ImGui::End();
    }
#endif

    return windowCount > 0;
}

int RunGui(int argc, char** argvUtf8) {
#ifdef HOOK_MAGIC_DESCRIPTION_GENERATOR
    void* handle = CreateFileMappingW(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 0x1000, L"SenMagicDescGenHook");
    if (handle != nullptr) {
        void* memory = MapViewOfFileEx(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000, nullptr);
        if (memory != nullptr) {
            s_memory = memory;
        }

        HyoutaUtils::IO::File f(
            std::string_view("g:\\SteamLibrary\\steamapps\\common\\The Legend of Heroes Trails "
                             "into Reverie\\dev\\data\\text\\dat_en\\t_magic.tbl"),
            HyoutaUtils::IO::OpenMode::Read);
        if (f.IsOpen()) {
            auto len = f.GetLength();
            if (len) {
                auto buffer = std::make_unique<char[]>(*len);
                if (buffer) {
                    if (f.Read(buffer.get(), *len) == *len) {
                        s_LoadedTbl = std::make_unique<SenLib::Sen5::Tbl>(buffer.get(), *len);
                    }
                }
            }
        }
    }
#endif

    GuiState state;
    std::optional<std::string> guiSettingsFolder =
        CommonPaths::GetLocalSenPatcherGuiSettingsFolder();
    std::string userIniPath;
    std::string imguiIniPath;
    if (guiSettingsFolder) {
        HyoutaUtils::IO::CreateDirectory(std::string_view(*guiSettingsFolder));
        userIniPath = *guiSettingsFolder;
        userIniPath.append("/gui.ini");
        imguiIniPath = *guiSettingsFolder;
        imguiIniPath.append("/imgui.ini");
        if (!SenTools::LoadUserSettingsFromIni(state.GuiSettings, userIniPath)) {
            SenTools::LoadUserSettingsFromCSharpUserConfig(state.GuiSettings, *guiSettingsFolder);
        }
    }
    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherMainWindow>());

    const auto load_imgui_ini = [&](ImGuiIO& io, GuiState& state) -> void {
        if (!guiSettingsFolder) {
            return;
        }
        HyoutaUtils::IO::File f(std::string_view(imguiIniPath), HyoutaUtils::IO::OpenMode::Read);
        if (!f.IsOpen()) {
            return;
        }
        auto length = f.GetLength();
        if (!length || *length == 0 || *length > (10 * 1024 * 1024)) {
            return;
        }
        const size_t bufferSize = static_cast<size_t>(*length);
        std::unique_ptr<char[]> buffer(new (std::nothrow) char[bufferSize]);
        if (!buffer) {
            return;
        }
        if (f.Read(buffer.get(), bufferSize) != bufferSize) {
            return;
        }
        ImGui::LoadIniSettingsFromMemory(buffer.get(), bufferSize);
    };
    const auto save_imgui_ini = [&](ImGuiIO& io, GuiState& state) -> void {
        if (!guiSettingsFolder) {
            return;
        }
        size_t length = 0;
        const char* buffer = ImGui::SaveIniSettingsToMemory(&length);
        if (!buffer) {
            return;
        }
        HyoutaUtils::IO::WriteFileAtomic(std::string_view(imguiIniPath), buffer, length);
    };

    const std::string_view windowTitle("SenPatcher " SENPATCHER_VERSION);
    ImVec4 backgroundColor(0.070f, 0.125f, 0.070f, 1.000f); // dark gray-ish green
    // ImVec4 backgroundColor(0.060f, 0.000f, 0.125f, 1.000f); // dark blue/purple
    // ImVec4 backgroundColor(0.000f, 0.000f, 0.000f, 1.000f); // black
#ifdef BUILD_FOR_WINDOWS
    auto wstr = HyoutaUtils::TextUtils::Utf8ToWString(windowTitle.data(), windowTitle.size());
    int rv = RunGuiDX11(state,
                        wstr ? wstr->c_str() : L"SenPatcher",
                        backgroundColor,
                        LoadFonts,
                        RenderFrame,
                        load_imgui_ini,
                        save_imgui_ini);
#else
    int rv = RunGuiGlfwVulkan(state,
                              windowTitle.data(),
                              backgroundColor,
                              LoadFonts,
                              RenderFrame,
                              load_imgui_ini,
                              save_imgui_ini);
#endif
    if (guiSettingsFolder) {
        HyoutaUtils::IO::CreateDirectory(std::string_view(*guiSettingsFolder));
        SenTools::WriteUserSettingsToIni(state.GuiSettings, userIniPath);
    }
    return rv;
}
} // namespace SenTools
