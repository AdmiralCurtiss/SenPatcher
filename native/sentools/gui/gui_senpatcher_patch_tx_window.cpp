#include "gui_senpatcher_patch_tx_window.h"

#include <atomic>
#include <charconv>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

#include "imgui.h"

#include "gui_senpatcher_asset_fix_window.h"
#include "gui_senpatcher_patch_window_utils.h"
#include "gui_state.h"
#include "sentools/senpatcher_dll_loader.h"
#include "sentools_imgui_utils.h"
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
    std::string LocalDllPath;
    bool Unpatch;

    HyoutaUtils::Result<bool, std::string> Result;
    std::atomic_bool IsDone = false;

    std::thread Thread;

    WorkThreadState(const Settings& gameSettings,
                    std::string gamePath,
                    std::string localDllPath,
                    bool unpatch)
      : GameSettings(gameSettings)
      , GamePath(std::move(gamePath))
      , LocalDllPath(std::move(localDllPath))
      , Unpatch(unpatch)
      , Result(false)
      , Thread([this]() -> void {
          auto doneGuard = HyoutaUtils::MakeScopeGuard([&]() { IsDone.store(true); });
          try {
              std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
              std::string gameIniPath = GamePath + RELATIVE_INI_PATH;
              std::string_view defaultIniString(IniData, IniLength);
              Result = SenTools::GUI::PatchOrUnpatchGame(
                  [&](HyoutaUtils::Ini::IniWriter& writer) { WriteToIni(GameSettings, writer); },
                  LocalDllPath,
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

void SenPatcherPatchTXWindow::Cleanup(GuiState& state) {
    state.WindowIdsPatchTX.ReturnId(WindowId);
}

SenPatcherPatchTXWindow::SenPatcherPatchTXWindow(GuiState& state,
                                                 std::string gamePath,
                                                 std::string patchDllPath,
                                                 HyoutaUtils::IO::File patchDllFile,
                                                 SenPatcherDllIdentificationResult patchDllInfo)
  : WindowId(GenerateWindowId(state.WindowIdsPatchTX,
                              WindowIdString.data(),
                              WindowIdString.size(),
                              WindowTitle,
                              sizeof(WindowTitle)))
  , GamePath(std::move(gamePath))
  , PatchDllPath(std::move(patchDllPath))
  , PatchDllFile(std::move(patchDllFile))
  , PatchDllInfo(std::move(patchDllInfo)) {
    UpdateInstalledDllInfo();

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

void SenPatcherPatchTXWindow::UpdateInstalledDllInfo() {
    std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
    InstalledDllInfo = SenTools::IdentifySenPatcherDll(gameDllPath);
}

SenPatcherPatchTXWindow::~SenPatcherPatchTXWindow() = default;

bool SenPatcherPatchTXWindow::RenderFrame(GuiState& state) {
    ImGuiUtils::SetInitialNextWindowSizeWidthOnly(600.0f * state.CurrentDpi);
    bool open = true;
    bool visible = ImGui::Begin(WindowIdString.data(), &open, ImGuiWindowFlags_None);
    auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (!visible) {
        return open || WorkThread;
    }

    {
        auto configScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (WorkThread) {
            ImGui::BeginDisabled();
        } else {
            configScope.Dispose();
        }

        ImGui::Text("Path: %s", GamePath.c_str());
        ImGui::Text("SenPatcher version currently installed: %s",
                    InstalledDllInfo.Type == SenPatcherDllIdentificationType::TXHook
                        ? (InstalledDllInfo.Version.has_value() ? InstalledDllInfo.Version->c_str()
                                                                : "Unknown")
                        : "None");
        ImGui::Text("SenPatcher version to be installed: %s",
                    PatchDllInfo.Version.has_value() ? PatchDllInfo.Version->c_str() : "Unknown");

        if (ImGuiUtils::ButtonFullWidth("Remove Patches / Restore Original",
                                        30.0f * state.CurrentDpi)
            && !WorkThread) {
            StatusMessage = "Unpatching...";
            WorkThread = std::make_unique<SenPatcherPatchTXWindow::WorkThreadState>(
                GameSettings, GamePath, PatchDllPath, true);
        }

        ImGui::Checkbox("Apply fixes and improvements to game assets",
                        &GameSettings.CheckBoxAssetPatches);
        if (PatchDllInfo.FileFixInfo.has_value()) {
            ImGui::SameLine();
            SenTools::GUI::HandleAssetFixDetailButton(state, *PatchDllInfo.FileFixInfo);
        }

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
        ImGuiUtils::GamepadNavigableHelperTooltip(
            "##TurboTip",
            "(?)",
            "The default button for Turbo Mode is L3 on controller and Left Ctrl on keyboard. This "
            "can be changed in the in-game Button Configuration options.",
            state.CurrentDpi * 350.0f,
            true);

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

        if (ImGuiUtils::ButtonFullWidth("Patch!", 40.0f * state.CurrentDpi) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchTXWindow::WorkThreadState>(
                GameSettings, GamePath, PatchDllPath, false);
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
        UpdateInstalledDllInfo();
    }

    static constexpr char closeLabel[] = "Close";
    float closeTextWidth = ImGui::CalcTextSize(closeLabel, nullptr, true).x;
    float closeButtonWidth = closeTextWidth + (ImGui::GetStyle().FramePadding.x * 2.0f);
    if (!StatusMessage.empty()) {
        float wrap = ImGui::GetContentRegionAvail().x - closeButtonWidth;
        ImGui::PushTextWrapPos(wrap <= FLT_MIN ? FLT_MIN : wrap);
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
        ImGui::PopTextWrapPos();
        ImGui::SameLine();
    }

    {
        auto scope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
        if (WorkThread) {
            ImGui::BeginDisabled();
        } else {
            scope.Dispose();
        }
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                             - closeButtonWidth);
        if (ImGui::Button(closeLabel)) {
            open = false;
        }
    }

    return open || WorkThread;
}
} // namespace SenTools::GUI
