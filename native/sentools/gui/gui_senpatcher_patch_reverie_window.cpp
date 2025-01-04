#include "gui_senpatcher_patch_reverie_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

#include "imgui.h"

#include "gui_senpatcher_patch_window_utils.h"
#include "gui_state.h"
#include "util/file.h"
#include "util/ini.h"
#include "util/ini_writer.h"
#include "util/result.h"
#include "util/scope.h"
#include "util/text.h"

namespace {
static constexpr char IniData[] = {
#include "ini_reverie.h"
};
static constexpr size_t IniLength = sizeof(IniData);

static constexpr char LOCAL_GAME_DIR[] = "The Legend of Heroes Trails into Reverie";
static constexpr char RELATIVE_DLL_PATH[] = "/bin/Win64/DSOUND.dll";
static constexpr char RELATIVE_INI_PATH[] = "/senpatcher_settings.ini";
} // namespace

namespace SenTools::GUI {
static void WriteToIni(const SenPatcherPatchReverieWindow::Settings& settings,
                       HyoutaUtils::Ini::IniWriter& writer) {
    writer.SetBool("Reverie", "AssetFixes", settings.CheckBoxAssetPatches);
    writer.SetBool("Reverie", "FixBgmEnqueue", settings.CheckBoxBgmEnqueueingLogic);
    writer.SetBool("Reverie", "DisableMouseCapture", settings.CheckBoxDisableMouseCam);
    writer.SetBool("Reverie", "ShowMouseCursor", settings.CheckBoxShowMouseCursor);
    writer.SetBool(
        "Reverie", "DisableFpsLimitOnFocusLoss", settings.CheckBoxDisableFpsLimitOnFocusLoss);
}

struct SenPatcherPatchReverieWindow::WorkThreadState {
    Settings GameSettings;
    std::string GamePath;
    bool Unpatch;

    HyoutaUtils::Result<bool, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    WorkThreadState(const Settings& gameSettings, std::string gamePath, bool unpatch)
      : GameSettings(gameSettings)
      , GamePath(std::move(gamePath))
      , Unpatch(unpatch)
      , Result(false)
      , Thread([this]() -> void {
          auto doneGuard = HyoutaUtils::MakeScopeGuard([&]() { IsDone.store(true); });
          try {
              std::string localDllPath = std::string(LOCAL_GAME_DIR) + RELATIVE_DLL_PATH;
              std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
              std::string gameIniPath = GamePath + RELATIVE_INI_PATH;
              std::string_view defaultIniString(IniData, IniLength);
              Result = SenTools::GUI::PatchOrUnpatchGame(
                  [&](HyoutaUtils::Ini::IniWriter& writer) { WriteToIni(GameSettings, writer); },
                  localDllPath,
                  gameDllPath,
                  gameIniPath,
                  defaultIniString,
                  Unpatch);
          } catch (...) {
              Result = std::string("Unexpected error.");
          }
      }) {}

    ~WorkThreadState() {
        if (Thread.joinable()) {
            Thread.join();
        }
    }
};

SenPatcherPatchReverieWindow::SenPatcherPatchReverieWindow(GuiState& state,
                                                           std::string_view gamePath)
  : GamePath(gamePath) {
    // TODO: Is there a better way to get imgui to handle windows where the user can create as many
    // copies as they want at will?
    sprintf(WindowID.data(), "%s##W%zx", WindowTitle, state.WindowIndexCounter++);

    // load config stored in game path, if any
    HyoutaUtils::Ini::IniFile ini;
    HyoutaUtils::IO::File f(std::string_view(GamePath + RELATIVE_INI_PATH),
                            HyoutaUtils::IO::OpenMode::Read);
    if (f.IsOpen() && ini.ParseFile(f)) {
        const auto check_boolean = [&](std::string_view section, std::string_view key, bool& b) {
            const auto* kvp = ini.FindValue(section, key);
            if (kvp) {
                using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                if (CaseInsensitiveEquals(kvp->Value, "true")) {
                    b = true;
                } else if (CaseInsensitiveEquals(kvp->Value, "false")) {
                    b = false;
                }
            }
        };
        const auto check_integer = [&](std::string_view section, std::string_view key, int& i) {
            const auto* kvp = ini.FindValue(section, key);
            if (kvp) {
                int intval = 0;
                const auto [_, ec] = std::from_chars(
                    kvp->Value.data(), kvp->Value.data() + kvp->Value.size(), intval);
                if (ec == std::errc()) {
                    i = intval;
                }
            }
        };
        check_boolean("Reverie", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_boolean("Reverie", "FixBgmEnqueue", GameSettings.CheckBoxBgmEnqueueingLogic);
        check_boolean("Reverie", "DisableMouseCapture", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("Reverie", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean("Reverie",
                      "DisableFpsLimitOnFocusLoss",
                      GameSettings.CheckBoxDisableFpsLimitOnFocusLoss);
    }
}

SenPatcherPatchReverieWindow::~SenPatcherPatchReverieWindow() = default;

static void HelpMarker(std::string_view desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool SenPatcherPatchReverieWindow::RenderFrame(GuiState& state) {
    ImGui::SetNextWindowSize(ImVec2(600, 550), ImGuiCond_Once);
    bool open = true;
    bool visible = ImGui::Begin(WindowID.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open;
    }

    {
        auto configScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (WorkThread) {
            ImGui::BeginDisabled();
        } else {
            configScope.Dispose();
        }

        ImGui::Text("Path: %s", GamePath.c_str());

        if (ImGui::Button("Remove Patches / Restore Original", ImVec2(-1.0f, 30.0f))
            && !WorkThread) {
            StatusMessage = "Unpatching...";
            WorkThread = std::make_unique<SenPatcherPatchReverieWindow::WorkThreadState>(
                GameSettings, GamePath, true);
        }

        ImGui::Checkbox("Apply fixes for known script/asset errors",
                        &GameSettings.CheckBoxAssetPatches);
        // ImGui::SameLine();
        // if (ImGui::Button("Show asset fix details (may contain spoilers)")) {
        //     // TODO
        // }
        ImGui::Checkbox("Fix wrong BGM when BGM is changed while track is fading out",
                        &GameSettings.CheckBoxBgmEnqueueingLogic);
        ImGui::Checkbox("Disable Mouse Capture and Mouse Camera",
                        &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Remove 30 FPS limit when in Background",
                        &GameSettings.CheckBoxDisableFpsLimitOnFocusLoss);

        if (ImGui::Button("Patch!", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchReverieWindow::WorkThreadState>(
                GameSettings, GamePath, false);
        }
    }

    if (WorkThread && WorkThread->IsDone.load()) {
        WorkThread->Thread.join();
        if (WorkThread->Result.IsError()) {
            StatusMessage = WorkThread->Result.GetErrorValue();
        } else {
            StatusMessage.clear();
        }
        WorkThread.reset();
    }

    if (!StatusMessage.empty()) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
        ImGui::PopTextWrapPos();
    }

    return open;
}
} // namespace SenTools::GUI
