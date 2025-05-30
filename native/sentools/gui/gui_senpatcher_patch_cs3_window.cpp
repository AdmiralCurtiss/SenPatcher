#include "gui_senpatcher_patch_cs3_window.h"

#include <atomic>
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
#include "ini_cs3.h"
};
static constexpr size_t IniLength = sizeof(IniData);

static constexpr char RELATIVE_DLL_PATH[] = "/bin/x64/DINPUT8.dll";
static constexpr char RELATIVE_INI_PATH[] = "/senpatcher_settings.ini";
} // namespace

namespace SenTools::GUI {
static void WriteToIni(const SenPatcherPatchCS3Window::Settings& settings,
                       HyoutaUtils::Ini::IniWriter& writer) {
    writer.SetBool("CS3", "AssetFixes", settings.CheckBoxAssetPatches);
    writer.SetBool("CS3", "MakeTurboToggle", settings.CheckBoxTurboToggle);
    writer.SetBool("CS3", "FixInGameButtonRemapping", settings.CheckBoxFixButtonRemapping);
    writer.SetBool("CS3", "AllowSwitchToNightmare", settings.CheckBoxAllowNightmare);
    writer.SetBool("CS3", "FixBgmEnqueue", settings.CheckBoxBgmEnqueueingLogic);
    writer.SetBool("CS3", "FixControllerMapping", settings.CheckBoxControllerMapping);
    writer.SetBool("CS3", "DisableMouseCapture", settings.CheckBoxDisableMouseCam);
    writer.SetBool("CS3", "ShowMouseCursor", settings.CheckBoxShowMouseCursor);
    writer.SetBool("CS3", "DisablePauseOnFocusLoss", settings.CheckBoxDisablePauseOnFocusLoss);
    writer.SetBool("CS3", "ForceXInput", settings.CheckBoxForceXInput);
    writer.SetBool("CS3", "DisableMovementMotionBlur", settings.CheckBoxDisableMotionBlur);
}

struct SenPatcherPatchCS3Window::WorkThreadState {
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

void SenPatcherPatchCS3Window::Cleanup(GuiState& state) {
    state.WindowIdsPatchCS3.ReturnId(WindowId);
}

SenPatcherPatchCS3Window::SenPatcherPatchCS3Window(GuiState& state,
                                                   std::string gamePath,
                                                   std::string patchDllPath,
                                                   HyoutaUtils::IO::File patchDllFile,
                                                   SenPatcherDllIdentificationResult patchDllInfo)
  : WindowId(GenerateWindowId(state.WindowIdsPatchCS3,
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
        check_boolean("CS3", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_boolean("CS3", "MakeTurboToggle", GameSettings.CheckBoxTurboToggle);
        check_boolean("CS3", "FixInGameButtonRemapping", GameSettings.CheckBoxFixButtonRemapping);
        check_boolean("CS3", "AllowSwitchToNightmare", GameSettings.CheckBoxAllowNightmare);
        check_boolean("CS3", "FixBgmEnqueue", GameSettings.CheckBoxBgmEnqueueingLogic);
        check_boolean("CS3", "FixControllerMapping", GameSettings.CheckBoxControllerMapping);
        check_boolean("CS3", "DisableMouseCapture", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("CS3", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean(
            "CS3", "DisablePauseOnFocusLoss", GameSettings.CheckBoxDisablePauseOnFocusLoss);
        check_boolean("CS3", "ForceXInput", GameSettings.CheckBoxForceXInput);
        check_boolean("CS3", "DisableMovementMotionBlur", GameSettings.CheckBoxDisableMotionBlur);
    }
}

void SenPatcherPatchCS3Window::UpdateInstalledDllInfo() {
    std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
    InstalledDllInfo = SenTools::IdentifySenPatcherDll(gameDllPath);
}

SenPatcherPatchCS3Window::~SenPatcherPatchCS3Window() = default;

bool SenPatcherPatchCS3Window::RenderFrame(GuiState& state) {
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
                    InstalledDllInfo.Type == SenPatcherDllIdentificationType::CS3Hook
                        ? (InstalledDllInfo.Version.has_value() ? InstalledDllInfo.Version->c_str()
                                                                : "Unknown")
                        : "None");
        ImGui::Text("SenPatcher version to be installed: %s",
                    PatchDllInfo.Version.has_value() ? PatchDllInfo.Version->c_str() : "Unknown");

        if (ImGuiUtils::ButtonFullWidth("Remove Patches / Restore Original",
                                        30.0f * state.CurrentDpi)
            && !WorkThread) {
            StatusMessage = "Unpatching...";
            WorkThread = std::make_unique<SenPatcherPatchCS3Window::WorkThreadState>(
                GameSettings, GamePath, PatchDllPath, true);
        }

        ImGui::Checkbox("Apply fixes and improvements to game assets",
                        &GameSettings.CheckBoxAssetPatches);
        if (PatchDllInfo.FileFixInfo.has_value()) {
            ImGui::SameLine();
            SenTools::GUI::HandleAssetFixDetailButton(state, *PatchDllInfo.FileFixInfo);
        }
        ImGui::Checkbox("Make turbo mode a toggle instead of hold",
                        &GameSettings.CheckBoxTurboToggle);
        ImGui::Checkbox("Fix in-game button remapping options",
                        &GameSettings.CheckBoxFixButtonRemapping);
        ImGui::Checkbox("Allow changing difficulty to Nightmare mid-playthough",
                        &GameSettings.CheckBoxAllowNightmare);
        ImGui::Checkbox(
            "Fix inconsistent Controller Button Prompts and Mappings when remapping buttons",
            &GameSettings.CheckBoxControllerMapping);
        ImGui::Checkbox("Fix wrong BGM when BGM is changed while track is fading out",
                        &GameSettings.CheckBoxBgmEnqueueingLogic);
        ImGui::Checkbox("Disable Mouse Capture and Mouse Camera",
                        &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Keep game running when in Background",
                        &GameSettings.CheckBoxDisablePauseOnFocusLoss);
        ImGui::Checkbox(
            "Force all controllers as XInput (may fix issues with newer Xbox controllers)",
            &GameSettings.CheckBoxForceXInput);
        ImGui::Checkbox("Disable the motion blur while running (backport from CS4/Reverie)",
                        &GameSettings.CheckBoxDisableMotionBlur);

        if (ImGuiUtils::ButtonFullWidth("Patch!", 40.0f * state.CurrentDpi) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchCS3Window::WorkThreadState>(
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
