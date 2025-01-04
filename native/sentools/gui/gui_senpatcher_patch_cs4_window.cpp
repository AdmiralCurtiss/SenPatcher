#include "gui_senpatcher_patch_cs4_window.h"

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
#include "ini_cs4.h"
};
static constexpr size_t IniLength = sizeof(IniData);

static constexpr const char* BUTTON_LAYOUTS[] = {
    "Xbox or English PlayStation Style (Confirm on bottom, Cancel on right)",
    "Nintendo or Japanese PlayStation Style (Confirm on right, Cancel on bottom)"};

static constexpr char LOCAL_GAME_DIR[] = "The Legend of Heroes Trails of Cold Steel IV";
static constexpr char RELATIVE_DLL_PATH[] = "/bin/Win64/DINPUT8.dll";
static constexpr char RELATIVE_INI_PATH[] = "/senpatcher_settings.ini";
} // namespace

namespace SenTools::GUI {
static void WriteToIni(const SenPatcherPatchCS4Window::Settings& settings,
                       HyoutaUtils::Ini::IniWriter& writer) {
    writer.SetBool("CS4", "AssetFixes", settings.CheckBoxAssetPatches);
    writer.SetBool("CS4", "AllowSwitchToNightmare", settings.CheckBoxAllowNightmare);
    writer.SetBool("CS4", "FixBgmEnqueue", settings.CheckBoxBgmEnqueueingLogic);
    writer.SetBool("CS4", "ForceConfirmCancel", settings.CheckBoxButtonLayout);
    writer.SetBool("CS4", "ForceConfirmJp", settings.ComboBoxForceJp);
    writer.SetBool("CS4", "DisableMouseCapture", settings.CheckBoxDisableMouseCam);
    writer.SetBool("CS4", "ShowMouseCursor", settings.CheckBoxShowMouseCursor);
    writer.SetBool("CS4", "DisablePauseOnFocusLoss", settings.CheckBoxDisablePauseOnFocusLoss);
}

struct SenPatcherPatchCS4Window::WorkThreadState {
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

SenPatcherPatchCS4Window::SenPatcherPatchCS4Window(GuiState& state, std::string_view gamePath)
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
        check_boolean("CS4", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_boolean("CS4", "AllowSwitchToNightmare", GameSettings.CheckBoxAllowNightmare);
        check_boolean("CS4", "FixBgmEnqueue", GameSettings.CheckBoxBgmEnqueueingLogic);
        check_boolean("CS4", "ForceConfirmCancel", GameSettings.CheckBoxButtonLayout);
        check_boolean("CS4", "ForceConfirmJp", GameSettings.ComboBoxForceJp);
        check_boolean("CS4", "DisableMouseCapture", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("CS4", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean(
            "CS4", "DisablePauseOnFocusLoss", GameSettings.CheckBoxDisablePauseOnFocusLoss);
    }
}

SenPatcherPatchCS4Window::~SenPatcherPatchCS4Window() = default;

static void HelpMarker(std::string_view desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool SenPatcherPatchCS4Window::RenderFrame(GuiState& state) {
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
            WorkThread = std::make_unique<SenPatcherPatchCS4Window::WorkThreadState>(
                GameSettings, GamePath, true);
        }

        ImGui::Checkbox("Apply fixes for known script/asset errors",
                        &GameSettings.CheckBoxAssetPatches);
        // ImGui::SameLine();
        // if (ImGui::Button("Show asset fix details (may contain spoilers)")) {
        //     // TODO
        // }
        ImGui::Checkbox("Allow changing difficulty to Nightmare mid-playthough",
                        &GameSettings.CheckBoxAllowNightmare);
        ImGui::Checkbox("Fix wrong BGM when BGM is changed while track is fading out",
                        &GameSettings.CheckBoxBgmEnqueueingLogic);

        ImGui::Checkbox("Force Confirm/Cancel button layout to:",
                        &GameSettings.CheckBoxButtonLayout);
        ImGui::SameLine();
        if (!GameSettings.CheckBoxButtonLayout) {
            ImGui::BeginDisabled();
        }
        if (ImGui::BeginCombo("",
                              BUTTON_LAYOUTS[GameSettings.ComboBoxForceJp ? 1 : 0],
                              ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLargest)) {
            for (int n = 0; n < IM_ARRAYSIZE(BUTTON_LAYOUTS); ++n) {
                const bool is_selected = ((GameSettings.ComboBoxForceJp ? 1 : 0) == n);
                if (ImGui::Selectable(BUTTON_LAYOUTS[n], is_selected)) {
                    GameSettings.ComboBoxForceJp = (n == 1);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (!GameSettings.CheckBoxButtonLayout) {
            ImGui::EndDisabled();
        }

        ImGui::Checkbox("Disable Mouse Capture and Mouse Camera",
                        &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Keep game running when in Background",
                        &GameSettings.CheckBoxDisablePauseOnFocusLoss);

        if (ImGui::Button("Patch!", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchCS4Window::WorkThreadState>(
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
