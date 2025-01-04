#include "gui_senpatcher_patch_tx_window.h"

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
#include "ini_tx.h"
};
static constexpr size_t IniLength = sizeof(IniData);

static constexpr const char* TURBO_FACTORS[] = {"2", "3", "4", "5", "6"};
static constexpr const char* GAME_LANGUAGES[] = {"Japanese / \xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E",
                                                 "English / \xE8\x8B\xB1\xE8\xAA\x9E"};

static constexpr char LOCAL_GAME_DIR[] = "Tokyo Xanadu eX+";
static constexpr char RELATIVE_DLL_PATH[] = "/DINPUT8.dll";
static constexpr char RELATIVE_INI_PATH[] = "/senpatcher_settings.ini";
} // namespace

namespace SenTools::GUI {
static void WriteToIni(const SenPatcherPatchTXWindow::Settings& settings,
                       HyoutaUtils::Ini::IniWriter& writer) {
    writer.SetBool("TX", "AssetFixes", settings.CheckBoxAssetPatches);
    writer.SetString(
        "TX", "Language", settings.ComboBoxGameLanguageEnglish ? "English" : "Japanese");
    writer.SetBool("TX", "SkipLogos", settings.CheckBoxSkipLogos);
    writer.SetBool("TX", "SkipAllMovies", settings.CheckBoxSkipMovies);
    writer.SetDouble("TX", "TurboModeFactor", settings.NumericUpDownTurboFactor);
    writer.SetBool("TX", "MakeTurboToggle", settings.CheckBoxTurboToggle);
    writer.SetBool("TX", "DisableMouseCamera", settings.CheckBoxDisableMouseCam);
    writer.SetBool("TX", "ShowMouseCursor", settings.CheckBoxShowMouseCursor);
    writer.SetBool(
        "TX", "EnableBackgroundControllerInput", settings.CheckBoxEnableBackgroundControllerInput);
    writer.SetBool("TX", "DisableCameraAutoCenter", settings.CheckBoxDisableCamAutoCenter);
    writer.SetBool("TX", "FixBgmResume", settings.CheckBoxFixBgmResume);
}

struct SenPatcherPatchTXWindow::WorkThreadState {
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

SenPatcherPatchTXWindow::SenPatcherPatchTXWindow(GuiState& state, std::string_view gamePath)
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
        const auto check_float = [&](std::string_view section, std::string_view key, float& f) {
            const auto* kvp = ini.FindValue(section, key);
            if (kvp) {
                float floatval = 0.0f;
                const auto [_, ec] = std::from_chars(
                    kvp->Value.data(), kvp->Value.data() + kvp->Value.size(), floatval);
                if (ec == std::errc()) {
                    f = floatval;
                }
            }
        };
        const auto check_language = [&](std::string_view section, std::string_view key, bool& b) {
            const auto* kvp = ini.FindValue(section, key);
            if (kvp) {
                using HyoutaUtils::TextUtils::CaseInsensitiveEquals;
                if (CaseInsensitiveEquals(kvp->Value, "Japanese")) {
                    b = false;
                }
            }
        };

        check_boolean("TX", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_language("TX", "Language", GameSettings.ComboBoxGameLanguageEnglish);
        check_boolean("TX", "SkipLogos", GameSettings.CheckBoxSkipLogos);
        check_boolean("TX", "SkipAllMovies", GameSettings.CheckBoxSkipMovies);
        check_boolean("TX", "MakeTurboToggle", GameSettings.CheckBoxTurboToggle);
        check_boolean("TX", "DisableMouseCamera", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("TX", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean("TX",
                      "EnableBackgroundControllerInput",
                      GameSettings.CheckBoxEnableBackgroundControllerInput);
        check_boolean("TX", "DisableCameraAutoCenter", GameSettings.CheckBoxDisableCamAutoCenter);
        check_boolean("TX", "FixBgmResume", GameSettings.CheckBoxFixBgmResume);

        float factor = 2.0f;
        check_float("TX", "TurboModeFactor", factor);
        if (factor >= 2.0f && factor <= 6.0f) {
            GameSettings.NumericUpDownTurboFactor = factor;
        }
    }
}

SenPatcherPatchTXWindow::~SenPatcherPatchTXWindow() = default;

static void HelpMarker(std::string_view desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool SenPatcherPatchTXWindow::RenderFrame(GuiState& state) {
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
            WorkThread = std::make_unique<SenPatcherPatchTXWindow::WorkThreadState>(
                GameSettings, GamePath, true);
        }

        ImGui::Checkbox("Apply fixes for known script/asset errors",
                        &GameSettings.CheckBoxAssetPatches);
        // ImGui::SameLine();
        // if (ImGui::Button("Show asset fix details (may contain spoilers)")) {
        //     // TODO
        // }

        const float factor = GameSettings.NumericUpDownTurboFactor;
        const int turboFactorIndex = factor >= 5.5f   ? 4
                                     : factor >= 4.5f ? 3
                                     : factor >= 3.5f ? 2
                                     : factor >= 2.5f ? 1
                                                      : 0;
        if (ImGui::BeginCombo("Turbo Factor",
                              TURBO_FACTORS[turboFactorIndex],
                              ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLargest)) {
            for (int n = 0; n < IM_ARRAYSIZE(TURBO_FACTORS); ++n) {
                const bool is_selected = (turboFactorIndex == n);
                if (ImGui::Selectable(TURBO_FACTORS[n], is_selected)) {
                    GameSettings.NumericUpDownTurboFactor = static_cast<float>(n + 2);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        HelpMarker(
            "The default button for Turbo Mode is L3 on controller and Left Ctrl on "
            "keyboard.\nThis can be changed in the in-game Button Configuration options.");

        ImGui::Checkbox("Make turbo mode a toggle instead of hold",
                        &GameSettings.CheckBoxTurboToggle);

        ImGui::Checkbox("Skip startup logos", &GameSettings.CheckBoxSkipLogos);
        ImGui::Checkbox("Skip all movies", &GameSettings.CheckBoxSkipMovies);
        ImGui::Checkbox("Fix music resuming bug", &GameSettings.CheckBoxFixBgmResume);
        ImGui::Checkbox("Disable camera auto-centering",
                        &GameSettings.CheckBoxDisableCamAutoCenter);

        ImGui::Checkbox("Disable Mouse Camera", &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Keep processing controller input when in Background",
                        &GameSettings.CheckBoxEnableBackgroundControllerInput);

        if (ImGui::BeginCombo("Game language",
                              GAME_LANGUAGES[GameSettings.ComboBoxGameLanguageEnglish ? 1 : 0],
                              ImGuiComboFlags_WidthFitPreview | ImGuiComboFlags_HeightLargest)) {
            for (int n = 0; n < IM_ARRAYSIZE(GAME_LANGUAGES); ++n) {
                const bool is_selected = ((GameSettings.ComboBoxGameLanguageEnglish ? 1 : 0) == n);
                if (ImGui::Selectable(GAME_LANGUAGES[n], is_selected)) {
                    GameSettings.ComboBoxGameLanguageEnglish = (n == 1);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Patch!", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchTXWindow::WorkThreadState>(
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
