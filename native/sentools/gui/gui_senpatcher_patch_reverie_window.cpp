#include "gui_senpatcher_patch_reverie_window.h"

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
#include "ini_reverie.h"
};
static constexpr size_t IniLength = sizeof(IniData);

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

void SenPatcherPatchReverieWindow::Cleanup(GuiState& state) {
    state.WindowIdsPatchReverie.ReturnId(WindowId);
}

SenPatcherPatchReverieWindow::SenPatcherPatchReverieWindow(
    GuiState& state,
    std::string gamePath,
    std::string patchDllPath,
    HyoutaUtils::IO::File patchDllFile,
    SenPatcherDllIdentificationResult patchDllInfo)
  : WindowId(GenerateWindowId(state.WindowIdsPatchReverie,
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
        check_boolean("Reverie", "AssetFixes", GameSettings.CheckBoxAssetPatches);
        check_boolean("Reverie", "FixBgmEnqueue", GameSettings.CheckBoxBgmEnqueueingLogic);
        check_boolean("Reverie", "DisableMouseCapture", GameSettings.CheckBoxDisableMouseCam);
        check_boolean("Reverie", "ShowMouseCursor", GameSettings.CheckBoxShowMouseCursor);
        check_boolean("Reverie",
                      "DisableFpsLimitOnFocusLoss",
                      GameSettings.CheckBoxDisableFpsLimitOnFocusLoss);
    }
}

void SenPatcherPatchReverieWindow::UpdateInstalledDllInfo() {
    std::string gameDllPath = GamePath + RELATIVE_DLL_PATH;
    InstalledDllInfo = SenTools::IdentifySenPatcherDll(gameDllPath);
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
                    InstalledDllInfo.Type == SenPatcherDllIdentificationType::ReverieHook
                        ? (InstalledDllInfo.Version.has_value() ? InstalledDllInfo.Version->c_str()
                                                                : "Unknown")
                        : "None");
        ImGui::Text("SenPatcher version to be installed: %s",
                    PatchDllInfo.Version.has_value() ? PatchDllInfo.Version->c_str() : "Unknown");

        if (ImGuiUtils::ButtonFullWidth("Remove Patches / Restore Original",
                                        30.0f * state.CurrentDpi)
            && !WorkThread) {
            StatusMessage = "Unpatching...";
            WorkThread = std::make_unique<SenPatcherPatchReverieWindow::WorkThreadState>(
                GameSettings, GamePath, PatchDllPath, true);
        }

        ImGui::Checkbox("Apply fixes and improvements to game assets",
                        &GameSettings.CheckBoxAssetPatches);
        if (PatchDllInfo.FileFixInfo.has_value()) {
            ImGui::SameLine();
            SenTools::GUI::HandleAssetFixDetailButton(state, *PatchDllInfo.FileFixInfo);
        }
        ImGui::Checkbox("Fix wrong BGM when BGM is changed while track is fading out",
                        &GameSettings.CheckBoxBgmEnqueueingLogic);
        ImGui::Checkbox("Disable Mouse Capture and Mouse Camera",
                        &GameSettings.CheckBoxDisableMouseCam);
        ImGui::Checkbox("Show Mouse Cursor", &GameSettings.CheckBoxShowMouseCursor);
        ImGui::Checkbox("Remove 30 FPS limit when in Background",
                        &GameSettings.CheckBoxDisableFpsLimitOnFocusLoss);

        if (ImGuiUtils::ButtonFullWidth("Patch!", 40.0f * state.CurrentDpi) && !WorkThread) {
            StatusMessage = "Patching...";
            WorkThread = std::make_unique<SenPatcherPatchReverieWindow::WorkThreadState>(
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
