#include "gui_senpatcher_patch_cs1_window.h"

#include <atomic>
#include <charconv>
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
#include "ini_cs1.h"
};
static constexpr size_t IniLength = sizeof(IniData);

static constexpr const char* CONTROLLER_BUTTONS[] = {"Square / X",
                                                     "Cross / A",
                                                     "Circle / B",
                                                     "Triangle / Y",
                                                     "L1 / LB",
                                                     "R1 / RB",
                                                     "L2 / LT",
                                                     "R2 / RT",
                                                     "Select / Back",
                                                     "Start",
                                                     "L3 / LS",
                                                     "R3 / RS",
                                                     "D-Pad Up",
                                                     "D-Pad Right",
                                                     "D-Pad Down",
                                                     "D-Pad Left"};

static constexpr char LOCAL_GAME_DIR[] = "Trails of Cold Steel";
static constexpr char RELATIVE_DLL_PATH[] = "/DINPUT8.dll";
static constexpr char RELATIVE_INI_PATH[] = "/senpatcher_settings.ini";
} // namespace

namespace SenTools::GUI {
static void WriteToIni(const SenPatcherPatchCS1Window::Settings& settings,
                       HyoutaUtils::Ini::IniWriter& writer) {
    writer.SetBool("CS1", "AssetFixes", settings.CheckBoxAssetPatches);
    writer.SetBool("CS1", "RemoveTurboSkip", settings.CheckBoxBattleAutoSkip);
    writer.SetBool("CS1", "MakeTurboToggle", settings.CheckBoxTurboToggle);
    writer.SetBool("CS1", "CorrectLanguageVoiceTables", settings.CheckBoxFixVoiceFileLang);
    writer.SetBool("CS1", "FixArtsSupportCutin", settings.CheckBoxArtsSupport);
    writer.SetBool("CS1", "Force0Kerning", settings.CheckBoxForce0Kerning);
    writer.SetBool("CS1", "DisableMouseCapture", settings.CheckBoxDisableMouseCam);
    writer.SetBool("CS1", "ShowMouseCursor", settings.CheckBoxShowMouseCursor);
    writer.SetBool("CS1", "DisablePauseOnFocusLoss", settings.CheckBoxDisablePauseOnFocusLoss);
    writer.SetBool("CS1", "ForceXInput", settings.CheckBoxForceXInput);
    writer.SetBool("CS1", "AlwaysUseNotebookR2", settings.CheckBoxAllowR2InTurboMode);
    writer.SetBool("CS1", "FixBgmEnqueue", settings.CheckBoxBgmEnqueueingLogic);
    writer.SetInt("CS1", "TurboModeButton", settings.ComboBoxTurboModeKey);
}

struct SenPatcherPatchCS1Window::WorkThreadState {
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

SenPatcherPatchCS1Window::SenPatcherPatchCS1Window(GuiState& state, std::string_view gamePath)
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
        check_boolean("CS1", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_boolean("CS1", "RemoveTurboSkip", GameSettings.CheckBoxBattleAutoSkip);
        check_boolean("CS1", "MakeTurboToggle", GameSettings.CheckBoxTurboToggle);
        check_boolean("CS1", "CorrectLanguageVoiceTables", GameSettings.CheckBoxFixVoiceFileLang);
        check_boolean("CS1", "FixArtsSupportCutin", GameSettings.CheckBoxArtsSupport);
        check_boolean("CS1", "Force0Kerning", GameSettings.CheckBoxForce0Kerning);
        check_boolean("CS1", "DisableMouseCapture", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("CS1", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean(
            "CS1", "DisablePauseOnFocusLoss", GameSettings.CheckBoxDisablePauseOnFocusLoss);
        check_boolean("CS1", "ForceXInput", GameSettings.CheckBoxForceXInput);
        check_boolean("CS1", "AlwaysUseNotebookR2", GameSettings.CheckBoxAllowR2InTurboMode);
        check_boolean("CS1", "FixBgmEnqueue", GameSettings.CheckBoxBgmEnqueueingLogic);
        int turboIndex = 7;
        check_integer("CS1", "TurboModeButton", turboIndex);
        if (turboIndex >= 0 && turboIndex < IM_ARRAYSIZE(CONTROLLER_BUTTONS)) {
            GameSettings.ComboBoxTurboModeKey = turboIndex;
        }
    }
}

SenPatcherPatchCS1Window::~SenPatcherPatchCS1Window() = default;

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool SenPatcherPatchCS1Window::RenderFrame(GuiState& state) {
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
            WorkThread = std::make_unique<SenPatcherPatchCS1Window::WorkThreadState>(
                GameSettings, GamePath, true);
        }

        ImGui::Checkbox("Apply fixes for known script/asset errors",
                        &GameSettings.CheckBoxAssetPatches);
        // ImGui::SameLine();
        // if (ImGui::Button("Show asset fix details (may contain spoilers)")) {
        //     // TODO
        // }
        ImGui::Checkbox("Remove animation skip in Turbo mode",
                        &GameSettings.CheckBoxBattleAutoSkip);
        ImGui::Checkbox("Make turbo mode a toggle instead of hold",
                        &GameSettings.CheckBoxTurboToggle);
        ImGui::Checkbox("Load voice metadata matching voice language rather than text language",
                        &GameSettings.CheckBoxFixVoiceFileLang);
        ImGui::Checkbox("Fix Arts Support cut-in issues when not running at 1280x720",
                        &GameSettings.CheckBoxArtsSupport);
        ImGui::Checkbox("Fix wrong BGM when BGM is changed while track is fading out",
                        &GameSettings.CheckBoxBgmEnqueueingLogic);
        ImGui::Checkbox(
            "Adjust font spacing for use with HD Cuprum font from CS3 or HD Texture Pack",
            &GameSettings.CheckBoxForce0Kerning);
        ImGui::Checkbox("Disable Mouse Capture and Mouse Camera",
                        &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Keep game running when in Background",
                        &GameSettings.CheckBoxDisablePauseOnFocusLoss);
        ImGui::Checkbox(
            "Force all controllers as XInput (may fix issues with newer Xbox controllers)",
            &GameSettings.CheckBoxForceXInput);
        ImGui::Checkbox("Enable R2 Notebook Shortcut when Turbo is enabled",
                        &GameSettings.CheckBoxAllowR2InTurboMode);

        if (ImGui::BeginCombo("Turbo mode key",
                              CONTROLLER_BUTTONS[GameSettings.ComboBoxTurboModeKey],
                              ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLargest)) {
            for (int n = 0; n < IM_ARRAYSIZE(CONTROLLER_BUTTONS); ++n) {
                const bool is_selected = (GameSettings.ComboBoxTurboModeKey == n);
                if (ImGui::Selectable(CONTROLLER_BUTTONS[n], is_selected)) {
                    GameSettings.ComboBoxTurboModeKey = n;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        HelpMarker(
            "All original functionality of the selected button will still trigger when pressing "
            "that button. L3/LS is unused in CS1, so you can map it there and then remap it to a "
            "more convenient button using eg. Steam's button remapping feature. Some buttons may "
            "be inaccessible when using Mouse/Keyboard controls.");

        if (ImGui::Button("Patch!", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchCS1Window::WorkThreadState>(
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
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
    }

    return open;
}
} // namespace SenTools::GUI
