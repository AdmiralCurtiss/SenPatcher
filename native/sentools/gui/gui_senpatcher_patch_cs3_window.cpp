#include "gui_senpatcher_patch_cs3_window.h"

#include <atomic>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

#include "imgui.h"

#include "gui_senpatcher_patch_window_utils.h"
#include "gui_state.h"
#include "sentools/senpatcher_dll_loader.h"
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

SenPatcherPatchCS3Window::SenPatcherPatchCS3Window(GuiState& state,
                                                   std::string gamePath,
                                                   std::string patchDllPath,
                                                   HyoutaUtils::IO::File patchDllFile,
                                                   SenPatcherDllIdentificationResult patchDllInfo)
  : GamePath(std::move(gamePath))
  , PatchDllPath(std::move(patchDllPath))
  , PatchDllFile(std::move(patchDllFile))
  , PatchDllInfo(std::move(patchDllInfo)) {
    UpdateInstalledDllInfo();

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
    }
}

void SenPatcherPatchCS3Window::UpdateInstalledDllInfo() {
    std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
    InstalledDllInfo = SenTools::IdentifySenPatcherDll(gameDllPath);
}

SenPatcherPatchCS3Window::~SenPatcherPatchCS3Window() = default;

static void HelpMarker(std::string_view desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool SenPatcherPatchCS3Window::RenderFrame(GuiState& state) {
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
        ImGui::Text("SenPatcher version currently installed: %s",
                    InstalledDllInfo.Type == SenPatcherDllIdentificationType::CS3Hook
                        ? (InstalledDllInfo.Version.has_value() ? InstalledDllInfo.Version->c_str()
                                                                : "Unknown")
                        : "None");
        ImGui::Text("SenPatcher version to be installed: %s",
                    PatchDllInfo.Version.has_value() ? PatchDllInfo.Version->c_str() : "Unknown");

        if (ImGui::Button("Remove Patches / Restore Original", ImVec2(-1.0f, 30.0f))
            && !WorkThread) {
            StatusMessage = "Unpatching...";
            WorkThread = std::make_unique<SenPatcherPatchCS3Window::WorkThreadState>(
                GameSettings, GamePath, PatchDllPath, true);
        }

        ImGui::Checkbox("Apply fixes for known script/asset errors",
                        &GameSettings.CheckBoxAssetPatches);
        if (PatchDllInfo.FileFixInfo.has_value()) {
            ImGui::SameLine();
            if (ImGui::Button("Show asset fix details (may contain spoilers)")) {
                ImGui::OpenPopup("Asset fix details");
            }
            bool modal_open = true;
            if (ImGui::BeginPopupModal(
                    "Asset fix details", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::TextUnformatted(PatchDllInfo.FileFixInfo->data(),
                                       PatchDllInfo.FileFixInfo->data()
                                           + PatchDllInfo.FileFixInfo->size());
                ImGui::EndPopup();
            }
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

        if (ImGui::Button("Patch!", ImVec2(-1.0f, 40.0f)) && !WorkThread) {
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

    if (!StatusMessage.empty()) {
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(StatusMessage.data(), StatusMessage.data() + StatusMessage.size());
        ImGui::PopTextWrapPos();
    }

    return open;
}
} // namespace SenTools::GUI
