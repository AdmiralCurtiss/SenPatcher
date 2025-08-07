#include "gui_senpatcher_main_window.h"

#include <array>
#include <condition_variable>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "imgui.h"

#include "dirtree/dirtree_cs1.h"
#include "dirtree/dirtree_cs2.h"
#include "dirtree/dirtree_cs3.h"
#include "dirtree/dirtree_cs4.h"
#include "dirtree/dirtree_reverie.h"
#include "dirtree/dirtree_tx.h"
#include "gui_senpatcher_compress_type1_window.h"
#include "gui_senpatcher_cs1_system_data_window.h"
#include "gui_senpatcher_cs2_system_data_window.h"
#include "gui_senpatcher_decompress_type1_window.h"
#include "gui_senpatcher_extract_p3a_window.h"
#include "gui_senpatcher_extract_pka_window.h"
#include "gui_senpatcher_extract_pkg_window.h"
#include "gui_senpatcher_fix_checksum_window.h"
#include "gui_senpatcher_patch_cs1_window.h"
#include "gui_senpatcher_patch_cs2_window.h"
#include "gui_senpatcher_patch_cs3_window.h"
#include "gui_senpatcher_patch_cs4_window.h"
#include "gui_senpatcher_patch_reverie_window.h"
#include "gui_senpatcher_patch_tx_window.h"
#include "gui_state.h"
#include "gui_user_settings.h"
#include "sen1/system_data.h"
#include "sen2/system_data.h"
#include "senpatcher_version.h"
#include "sentools/common_paths.h"
#include "sentools/cs2_14.h"
#include "sentools/game_verify/game_verify.h"
#include "sentools/old_senpatcher_unpatch.h"
#include "sentools/senpatcher_dll_loader.h"
#include "sentools_imgui_utils.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/scope.h"
#include "util/system.h"

namespace SenTools::GUI {
struct SenPatcherMainWindow::WorkThreadState {
    std::atomic_bool CancelAll = false;
    std::atomic_bool IsDone = false;
    std::atomic_bool Success = false;

    enum class UserInputRequestType {
        None,
        ErrorMessage,
        YesNoQuestion,
        YesNoYesToAllNoToAllQuestion,
    };
    enum class UserInputReplyType {
        None,
        OK,
        Yes,
        No,
        YesToAll,
        NoToAll,
    };
    UserInputRequestType UserInputRequest = UserInputRequestType::None;
    UserInputReplyType UserInputReply = UserInputReplyType::None;
    std::string UserInputRequestMessage;
    std::mutex UserInputRequestMutex;
    std::condition_variable UserInputRequestCondVar;

    std::optional<std::thread> Thread;

    std::string PathToOpen;
    std::string PatchDllPath;
    HyoutaUtils::IO::File PatchDllFile;
    SenPatcherDllIdentificationResult PatchDllInfo;

    // Shows an error message in a modal message box and waits for the user to confirm it.
    UserInputReplyType ShowError(std::string errorMessage) {
        std::unique_lock lk(UserInputRequestMutex);
        UserInputRequest = UserInputRequestType::ErrorMessage;
        UserInputReply = UserInputReplyType::None;
        UserInputRequestMessage = std::move(errorMessage);
        UserInputRequestCondVar.wait(
            lk, [this] { return CancelAll.load() || UserInputReply != UserInputReplyType::None; });
        return UserInputReply;
    }

    // Asks the user a yes/no question, waits for the reply, and returns.
    // If the message box is closed we assume a 'no' answer.
    UserInputReplyType ShowYesNoQuestion(std::string message) {
        std::unique_lock lk(UserInputRequestMutex);
        UserInputRequest = UserInputRequestType::YesNoQuestion;
        UserInputReply = UserInputReplyType::None;
        UserInputRequestMessage = std::move(message);
        UserInputRequestCondVar.wait(
            lk, [this] { return CancelAll.load() || UserInputReply != UserInputReplyType::None; });
        return UserInputReply;
    }

    // Asks the user a yes/no/yes-to-all/no-to-all question, waits for the reply, and returns.
    // If the message box is closed we assume a 'no' answer.
    UserInputReplyType ShowYesAllNoAllQuestion(std::string message) {
        std::unique_lock lk(UserInputRequestMutex);
        UserInputRequest = UserInputRequestType::YesNoYesToAllNoToAllQuestion;
        UserInputReply = UserInputReplyType::None;
        UserInputRequestMessage = std::move(message);
        UserInputRequestCondVar.wait(
            lk, [this] { return CancelAll.load() || UserInputReply != UserInputReplyType::None; });
        return UserInputReply;
    }

    ~WorkThreadState() {
        CancelAll.store(true);
        UserInputRequestCondVar.notify_all();
        if (Thread && Thread->joinable()) {
            Thread->join();
        }
    }

    bool DoUnpatchWithUserConfirmation(const std::string& path, int sengame) {
        if (SenPatcher::HasOldSenpatcherBackups(path, sengame)) {
            UserInputReplyType result = ShowYesNoQuestion(
                "Old SenPatcher backup files have been detected.\nThis version of SenPatcher "
                "requires an unpatched game to work correctly, so any existing patches need to be "
                "removed first.\n\nWould you like to remove any remaining old SenPatcher patches "
                "before proceeding? This may take a few seconds.\n\n"
                "(This may remove some non-SenPatcher mods you may have installed as well.)");
            if (result == UserInputReplyType::Yes) {
                if (!SenPatcher::UnpatchGame(path, sengame)) {
                    ShowError(
                        "Not all patches could be removed.\n\nYou should run a file verification "
                        "through Steam or GOG Galaxy before proceeding.");
                    return false;
                }
                return true;
            } else {
                return false;
            }
        }
        return true;
    }

    void DoFixUnicodeFilenames(const std::string& path,
                               const HyoutaUtils::DirTree::Tree& dirtree,
                               uint32_t gameVersionBits) {
        bool yesToAll = false;
        bool noToAll = false;

        SenTools::GameVerify::FixUnicodeFilenames(
            gameVersionBits, dirtree, path, [&](std::string_view path, size_t count) -> bool {
                if (yesToAll) {
                    return true;
                }
                if (noToAll) {
                    return false;
                }

                std::string question;
                if (count == 1) {
                    question = std::format(
                        "Found a {} with unicode encoding issues:\n{}\n\n"
                        "Would you like SenPatcher to fix this?",
                        HyoutaUtils::IO::DirectoryExists(path)
                                == HyoutaUtils::IO::ExistsResult::DoesExist
                            ? "directory"
                            : "file",
                        path);
                } else {
                    question = std::format(
                        "Found a directory containing {} files with unicode encoding "
                        "issues:\n{}\n\n"
                        "Would you like SenPatcher to fix this?",
                        count,
                        path);
                }

                UserInputReplyType result = ShowYesAllNoAllQuestion(question);
                if (result == UserInputReplyType::YesToAll) {
                    yesToAll = true;
                    return true;
                }
                if (result == UserInputReplyType::NoToAll) {
                    noToAll = true;
                    return false;
                }
                return result == UserInputReplyType::Yes;
            });
    }
};

SenPatcherMainWindow::SenPatcherMainWindow() = default;
SenPatcherMainWindow::~SenPatcherMainWindow() = default;

void SenPatcherMainWindow::Cleanup(GuiState& state) {}

bool SenPatcherMainWindow::RenderFrame(GuiState& state) {
    bool open = true;
    bool visible = ImGui::Begin("SenPatcher", &open, ImGuiWindowFlags_MenuBar);
    const auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (visible) {
        open = RenderContents(state) && open;
    }
    if (HasPendingWindowRequest()) {
        HandlePendingWindowRequest(state);
    }
    if (HasPendingWindowRequest()) {
        return true; // never close while something is still pending
    }
    return open;
}

bool SenPatcherMainWindow::HasPendingWindowRequest() const {
    return PendingWindowRequest != PendingWindowType::None;
}

bool SenPatcherMainWindow::RenderContents(GuiState& state) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Toolbox")) {
            if (ImGui::MenuItem("Extract P3A...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherExtractP3AWindow>(state));
            }
            if (ImGui::MenuItem("Extract PKG...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherExtractPkgWindow>(state));
            }
            if (ImGui::MenuItem("Extract PKA...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherExtractPkaWindow>(state));
            }
            if (ImGui::MenuItem("Fix Checksum of CS4 or Reverie save...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherFixChecksumWindow>(state));
            }
            if (ImGui::MenuItem("Decompress 'type 1' compressed file...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherDecompressType1Window>(state));
            }
            if (ImGui::MenuItem("Compress as 'type 1' compressed file...")) {
                state.Windows.emplace_back(
                    std::make_unique<GUI::SenPatcherCompressType1Window>(state));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Config")) {
            ImGui::MenuItem("Swap Confirm/Cancel on Controller",
                            nullptr,
                            &state.GuiSettings.GamepadSwapConfirmCancel);
            if (ImGui::BeginMenu("File Browser")) {
                if (ImGui::MenuItem("Use file browser from OS if possible",
                                    nullptr,
                                    state.GuiSettings.UseCustomFileBrowser
                                        == GuiUserSettings_UseCustomFileBrowser::Never)) {
                    state.GuiSettings.UseCustomFileBrowser =
                        GuiUserSettings_UseCustomFileBrowser::Never;
                }
                if (ImGui::MenuItem("Use custom file browser",
                                    nullptr,
                                    state.GuiSettings.UseCustomFileBrowser
                                        == GuiUserSettings_UseCustomFileBrowser::Always)) {
                    state.GuiSettings.UseCustomFileBrowser =
                        GuiUserSettings_UseCustomFileBrowser::Always;
                }
                if (ImGui::MenuItem("Autodetect",
                                    nullptr,
                                    state.GuiSettings.UseCustomFileBrowser
                                        == GuiUserSettings_UseCustomFileBrowser::Auto)) {
                    state.GuiSettings.UseCustomFileBrowser =
                        GuiUserSettings_UseCustomFileBrowser::Auto;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGuiUtils::TextUnformattedRightAlign("SenPatcher " SENPATCHER_VERSION);

    ImGui::Spacing();

    auto disabledScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
    if (HasPendingWindowRequest()) {
        ImGui::BeginDisabled();
    } else {
        disabledScope.Dispose();
    }

    ImGuiUtils::TextUnformatted("Trails of Cold Steel: (XSEED PC release version 1.6)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##1") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(
            FileFilter{"CS1 game directory root (Sen1Launcher.exe)", "Sen1Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          SenTools::GetDefaultPathCS1(state.GuiSettings),
                          "Sen1Launcher.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS1 game directory root (Sen1Launcher.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    bool modal_open = true;
    if (ImGui::BeginPopupModal("Select CS1 game directory root (Sen1Launcher.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS1 game directory root (Sen1Launcher.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS1Patch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGuiUtils::ButtonFullWidth("Edit System Save Data##1") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS1 System Data file (save511.dat)", "save511.dat"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        auto savedGames = SenTools::CommonPaths::GetSavedGamesFolder();
        std::string saveFolder;
        if (savedGames) {
            saveFolder = (*savedGames + "/FALCOM/ed8");
        }
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          saveFolder,
                          "save511.dat",
                          std::move(filters),
                          "dat",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS1 System Data file (save511.dat)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select CS1 System Data file (save511.dat)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS1 System Data file (save511.dat)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS1SystemData;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    ImGuiUtils::TextUnformatted(
        "Trails of Cold Steel II: (XSEED PC release version 1.4, 1.4.1, or 1.4.2)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##2") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(
            FileFilter{"CS2 game directory root (Sen2Launcher.exe)", "Sen2Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          SenTools::GetDefaultPathCS2(state.GuiSettings),
                          "Sen2Launcher.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS2 game directory root (Sen2Launcher.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select CS2 game directory root (Sen2Launcher.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS2 game directory root (Sen2Launcher.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS2Patch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGuiUtils::ButtonFullWidth("Edit System Save Data##2") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS2 System Data file (save255.dat)", "save255.dat"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        auto savedGames = SenTools::CommonPaths::GetSavedGamesFolder();
        std::string saveFolder;
        if (savedGames) {
            saveFolder = (*savedGames + "/FALCOM/ed8_2");
        }
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          saveFolder,
                          "save255.dat",
                          std::move(filters),
                          "dat",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS2 System Data file (save255.dat)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select CS2 System Data file (save255.dat)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS2 System Data file (save255.dat)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS2SystemData;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    ImGuiUtils::TextUnformatted("Trails of Cold Steel III: (NISA PC release version 1.06 or 1.07)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##3") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(
            FileFilter{"CS3 game directory root (Sen3Launcher.exe)", "Sen3Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          SenTools::GetDefaultPathCS3(state.GuiSettings),
                          "Sen3Launcher.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS3 game directory root (Sen3Launcher.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select CS3 game directory root (Sen3Launcher.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS3 game directory root (Sen3Launcher.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS3Patch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    ImGuiUtils::TextUnformatted(
        "Trails of Cold Steel IV: (NISA PC release version 1.2.1 or 1.2.2)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##4") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(
            FileFilter{"CS4 game directory root (Sen4Launcher.exe)", "Sen4Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          SenTools::GetDefaultPathCS4(state.GuiSettings),
                          "Sen4Launcher.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select CS4 game directory root (Sen4Launcher.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select CS4 game directory root (Sen4Launcher.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select CS4 game directory root (Sen4Launcher.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::CS4Patch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    ImGuiUtils::TextUnformatted("Trails into Reverie: (NISA PC release version 1.1.4 or 1.1.5)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##5") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"Reverie game directory (hnk.exe)", "hnk.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        std::string reverieDir = SenTools::GetDefaultPathReverie(state.GuiSettings);
        if (!reverieDir.empty()) {
            reverieDir += "/bin/Win64";
        }
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          reverieDir,
                          "hnk.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select Reverie game directory (hnk.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select Reverie game directory (hnk.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select Reverie game directory (hnk.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::ReveriePatch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    ImGuiUtils::TextUnformatted("Tokyo Xanadu eX+: (Aksys PC release version 1.08)");
    if (ImGuiUtils::ButtonFullWidth("Patch game##X") && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(
            FileFilter{"TX game directory root (TokyoXanadu.exe)", "TokyoXanadu.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          SenTools::GetDefaultPathTX(state.GuiSettings),
                          "TokyoXanadu.exe",
                          std::move(filters),
                          "exe",
                          false,
                          false,
                          SenTools::EvalUseCustomFileBrowser(state.GuiSettings));
        ImGui::OpenPopup("Select TX game directory root (TokyoXanadu.exe)");
    }
    ImGuiUtils::SetNextWindowSizeForNearFullscreenPopup();
    modal_open = true;
    if (ImGui::BeginPopupModal("Select TX game directory root (TokyoXanadu.exe)",
                               &modal_open,
                               ImGuiWindowFlags_NoSavedSettings)) {
        FileBrowserResult result =
            GameBrowser.RenderFrame(state, "Select TX game directory root (TokyoXanadu.exe)");
        if (result != FileBrowserResult::None) {
            if (result == FileBrowserResult::FileSelected) {
                PendingWindowRequest = PendingWindowType::TXPatch;
                PendingWindowSelectedPath = GameBrowser.GetSelectedPath();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGuiUtils::ButtonRightAlign("Close")) {
        return false;
    }

    return true;
}

void SenPatcherMainWindow::HandlePendingWindowRequest(GuiState& state) {
    if (WorkThread) {
        std::unique_lock lk(WorkThread->UserInputRequestMutex);
        switch (WorkThread->UserInputRequest) {
            case SenPatcherMainWindow::WorkThreadState::UserInputRequestType::None: break;
            case SenPatcherMainWindow::WorkThreadState::UserInputRequestType::ErrorMessage: {
                if (!ImGui::IsPopupOpen("Error")) {
                    ImGui::OpenPopup("Error");
                }
                ImGuiUtils::SetNextWindowSizeForStandardPopup();
                bool modal_open = true;
                if (ImGui::BeginPopupModal(
                        "Error", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    ImGuiUtils::TextUnformatted(WorkThread->UserInputRequestMessage);
                    ImGui::PopTextWrapPos();
                    if (ImGuiUtils::ButtonFullWidth("OK")) {
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    ImGui::EndPopup();
                }
                if (!modal_open) {
                    WorkThread->UserInputRequest =
                        SenPatcherMainWindow::WorkThreadState::UserInputRequestType::None;
                    WorkThread->UserInputReply =
                        SenPatcherMainWindow::WorkThreadState::UserInputReplyType::OK;
                    WorkThread->UserInputRequestCondVar.notify_all();
                }
                break;
            }
            case SenPatcherMainWindow::WorkThreadState::UserInputRequestType::YesNoQuestion: {
                if (!ImGui::IsPopupOpen("Question")) {
                    ImGui::OpenPopup("Question");
                }
                ImGuiUtils::SetNextWindowSizeForStandardPopup();
                bool modal_open = true;
                if (ImGui::BeginPopupModal(
                        "Question", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    ImGuiUtils::TextUnformatted(WorkThread->UserInputRequestMessage);
                    ImGui::PopTextWrapPos();
                    if (ImGuiUtils::ButtonFullWidth("Yes")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    if (ImGuiUtils::ButtonFullWidth("No")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::No;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    ImGui::EndPopup();
                }
                if (!modal_open) {
                    WorkThread->UserInputRequest =
                        SenPatcherMainWindow::WorkThreadState::UserInputRequestType::None;
                    if (WorkThread->UserInputReply
                        == SenPatcherMainWindow::WorkThreadState::UserInputReplyType::None) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::No;
                    }
                    WorkThread->UserInputRequestCondVar.notify_all();
                }
                break;
            }
            case SenPatcherMainWindow::WorkThreadState::UserInputRequestType::
                YesNoYesToAllNoToAllQuestion: {
                if (!ImGui::IsPopupOpen("Question")) {
                    ImGui::OpenPopup("Question");
                }
                ImGuiUtils::SetNextWindowSizeForStandardPopup();
                bool modal_open = true;
                if (ImGui::BeginPopupModal(
                        "Question", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    ImGuiUtils::TextUnformatted(WorkThread->UserInputRequestMessage);
                    ImGui::PopTextWrapPos();
                    if (ImGuiUtils::ButtonFullWidth("Yes")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    if (ImGuiUtils::ButtonFullWidth("No")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::No;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    if (ImGuiUtils::ButtonFullWidth("Yes to all")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::YesToAll;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    if (ImGuiUtils::ButtonFullWidth("No to all")) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::NoToAll;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    ImGui::EndPopup();
                }
                if (!modal_open) {
                    WorkThread->UserInputRequest =
                        SenPatcherMainWindow::WorkThreadState::UserInputRequestType::None;
                    if (WorkThread->UserInputReply
                        == SenPatcherMainWindow::WorkThreadState::UserInputReplyType::None) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::No;
                    }
                    WorkThread->UserInputRequestCondVar.notify_all();
                }
                break;
            }
            default: assert(0); break;
        }
    }

    switch (PendingWindowRequest) {
        default: {
            PendingWindowRequest = PendingWindowType::None;
            break;
        }
        case PendingWindowType::CS1Patch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath =
                            *currentExeDir + "/Trails of Cold Steel/DINPUT8.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::CS1Hook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for CS1. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        bool alreadyAsked = false;
                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);
                        if (SenTools::GameVerify::VerifyGame(
                                SenLib::Sen1::GetDirTree(),
                                path,
                                GameVerify::VerifyMode::IdentifyDirtree,
                                nullptr)
                            == 0) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be CS1. Correct patching "
                                    "behavior cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                            alreadyAsked = true;
                        }

                        threadState->DoUnpatchWithUserConfirmation(path, 1);

                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::Sen1::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!alreadyAsked && !(possibleVersions == (1 << 7))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of CS1 "
                                    "XSEED PC version 1.6. Correct patching behavior cannot be "
                                    "guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->DoFixUnicodeFilenames(
                            path, SenLib::Sen1::GetDirTree(), possibleVersions);

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.Sen1Path = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchCS1Window>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
        case PendingWindowType::CS1SystemData: {
            std::string_view selectedPath = PendingWindowSelectedPath;
            HyoutaUtils::IO::File f(selectedPath, HyoutaUtils::IO::OpenMode::Read);
            std::array<char, SenLib::Sen1::SystemData::FileLength> systemData;
            SenLib::Sen1::SystemData systemDataStruct;
            if (f.IsOpen() && f.GetLength() == systemData.size()
                && f.Read(systemData.data(), systemData.size()) == systemData.size()
                && systemDataStruct.Deserialize(systemData.data(), systemData.size())) {
                state.Windows.emplace_back(std::make_unique<GUI::SenPatcherCS1SystemDataWindow>(
                    state, selectedPath, systemDataStruct));
            }
            PendingWindowRequest = PendingWindowType::None;
            break;
        }
        case PendingWindowType::CS2Patch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath =
                            *currentExeDir + "/Trails of Cold Steel II/bin/Win32/DINPUT8.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::CS2Hook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for CS2. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        bool alreadyAsked = false;
                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);
                        if (SenTools::GameVerify::VerifyGame(
                                SenLib::Sen2::GetDirTree(),
                                path,
                                GameVerify::VerifyMode::IdentifyDirtree,
                                nullptr)
                            == 0) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be CS2. Correct patching "
                                    "behavior cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                            alreadyAsked = true;
                        }

                        auto updateResult = SenTools::TryPatchCs2Version14(path, [&]() -> bool {
                            return threadState->ShowYesNoQuestion(
                                       "This appears to be version 1.4 of Trails of Cold Steel "
                                       "II.\nSenPatcher does not support this version "
                                       "directly, but it can update the game to version 1.4.2, "
                                       "which is supported.\n\nWould you like to perform this "
                                       "update?")
                                   == SenPatcherMainWindow::WorkThreadState::UserInputReplyType::
                                       Yes;
                        });
                        if (updateResult == TryPatchCs2Version14Result::UpdateDeclined) {
                            return;
                        }
                        if (updateResult == TryPatchCs2Version14Result::WritingNewFilesFailed) {
                            threadState->ShowError(
                                "Patching failed. This shouldn't happen, either the game files or "
                                "SenPatcher itself is corrupted.");
                            return;
                        }
                        if (updateResult == TryPatchCs2Version14Result::WritingNewFilesFailed) {
                            threadState->ShowError(
                                "Writing the updated files failed.\n\nPlease ensure nothing is "
                                "preventing writes to the game files and try again.");
                            return;
                        }

                        threadState->DoUnpatchWithUserConfirmation(path, 2);

                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::Sen2::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!alreadyAsked
                            && !(possibleVersions == (1 << 6) || possibleVersions == (1 << 7))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of CS2 "
                                    "XSEED PC version 1.4.1 or 1.4.2. Correct patching behavior "
                                    "cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->DoFixUnicodeFilenames(
                            path, SenLib::Sen2::GetDirTree(), possibleVersions);

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.Sen2Path = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchCS2Window>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
        case PendingWindowType::CS2SystemData: {
            std::string_view selectedPath = PendingWindowSelectedPath;
            HyoutaUtils::IO::File f(selectedPath, HyoutaUtils::IO::OpenMode::Read);
            std::array<char, SenLib::Sen2::SystemData::FileLength> systemData;
            SenLib::Sen2::SystemData systemDataStruct;
            if (f.IsOpen() && f.GetLength() == systemData.size()
                && f.Read(systemData.data(), systemData.size()) == systemData.size()
                && systemDataStruct.Deserialize(systemData.data(), systemData.size())) {
                state.Windows.emplace_back(std::make_unique<GUI::SenPatcherCS2SystemDataWindow>(
                    state, selectedPath, systemDataStruct));
            }
            PendingWindowRequest = PendingWindowType::None;
            break;
        }
        case PendingWindowType::CS3Patch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath =
                            *currentExeDir
                            + "/The Legend of Heroes Trails of Cold Steel III/bin/x64/DINPUT8.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::CS3Hook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for CS3. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        bool alreadyAsked = false;
                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);
                        if (SenTools::GameVerify::VerifyGame(
                                SenLib::Sen3::GetDirTree(),
                                path,
                                GameVerify::VerifyMode::IdentifyDirtree,
                                nullptr)
                            == 0) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be CS3. Correct patching "
                                    "behavior cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                            alreadyAsked = true;
                        }

                        threadState->DoUnpatchWithUserConfirmation(path, 3);

                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::Sen3::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!alreadyAsked
                            && !(possibleVersions == (1 << 3) || possibleVersions == (1 << 4))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of CS3 "
                                    "NISA PC version 1.06 or 1.07. Correct patching behavior "
                                    "cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.Sen3Path = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchCS3Window>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
        case PendingWindowType::CS4Patch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath =
                            *currentExeDir
                            + "/The Legend of Heroes Trails of Cold Steel IV/bin/Win64/DINPUT8.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::CS4Hook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for CS4. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        bool alreadyAsked = false;
                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);
                        if (SenTools::GameVerify::VerifyGame(
                                SenLib::Sen4::GetDirTree(),
                                path,
                                GameVerify::VerifyMode::IdentifyDirtree,
                                nullptr)
                            == 0) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be CS4. Correct patching "
                                    "behavior cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                            alreadyAsked = true;
                        }

                        threadState->DoUnpatchWithUserConfirmation(path, 4);

                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::Sen4::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!alreadyAsked
                            && !(possibleVersions == (1 << 4) || possibleVersions == (1 << 5))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of CS4 "
                                    "NISA PC version 1.2.1 or 1.2.2. Correct patching behavior "
                                    "cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.Sen4Path = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchCS4Window>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
        case PendingWindowType::ReveriePatch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath =
                            *currentExeDir
                            + "/The Legend of Heroes Trails into Reverie/bin/Win64/DSOUND.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::ReverieHook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for Trails into Reverie. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(
                                             HyoutaUtils::IO::SplitPath(
                                                 HyoutaUtils::IO::SplitPath(selectedPath).Directory)
                                                 .Directory)
                                             .Directory);

                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::Sen5::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!(possibleVersions == (1 << 7) || possibleVersions == (1 << 8))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of "
                                    "Reverie NISA PC version 1.1.4 or 1.1.5. Correct patching "
                                    "behavior cannot be guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.Sen5Path = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchReverieWindow>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
        case PendingWindowType::TXPatch: {
            if (!WorkThread) {
                WorkThread = std::make_unique<SenPatcherMainWindow::WorkThreadState>();
                auto* threadState = WorkThread.get();
                WorkThread->Thread.emplace([threadState,
                                            selectedPath = PendingWindowSelectedPath]() -> void {
                    auto doneGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { threadState->IsDone.store(true); });
                    try {
                        auto currentExeDir = HyoutaUtils::Sys::GetCurrentExecutableDirectory();
                        if (!currentExeDir) {
                            // fall back to the current working directory
                            currentExeDir.emplace(".");
                        }
                        std::string patchDllPath = *currentExeDir + "/Tokyo Xanadu eX+/DINPUT8.dll";

                        HyoutaUtils::IO::File patchDllFile(std::string_view(patchDllPath),
                                                           HyoutaUtils::IO::OpenMode::Read);
                        if (!patchDllFile.IsOpen()) {
                            threadState->ShowError(
                                "Could not open the patch DLL file at '" + patchDllPath
                                + "'. Please ensure you've extracted SenPatcher correctly.");
                            return;
                        }
                        auto dllInfo = SenTools::IdentifySenPatcherDll(patchDllFile);
                        if (dllInfo.Type != SenPatcherDllIdentificationType::TXHook) {
                            threadState->ShowError(
                                "The file at '" + patchDllPath + "' does not appear to be a "
                                "SenPatcher DLL for Tokyo Xanadu. "
                                "Please redownload SenPatcher and try again.");
                            return;
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);
                        uint32_t possibleVersions = SenTools::GameVerify::VerifyGame(
                            SenLib::TX::GetDirTree(),
                            path,
                            GameVerify::VerifyMode::ExecutablesOnly,
                            nullptr);
                        if (!(possibleVersions == (1 << 3) || possibleVersions == (1 << 4))) {
                            if (threadState->ShowYesNoQuestion(
                                    "The selected directory not appear to be a clean copy of "
                                    "Tokyo Xanadu Aksys PC version 1.08 (Steam or GOG build). "
                                    "Correct patching behavior cannot be guaranteed. Proceed "
                                    "anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        threadState->PathToOpen = std::move(path);
                        threadState->PatchDllPath = std::move(patchDllPath);
                        threadState->PatchDllFile = std::move(patchDllFile);
                        threadState->PatchDllInfo = std::move(dllInfo);
                        threadState->Success.store(true);
                    } catch (...) {
                        threadState->ShowError("Unexpected error while opening window.");
                    }
                });
            }
            if (WorkThread && WorkThread->IsDone.load()) {
                WorkThread->Thread->join();
                if (WorkThread->Success.load()) {
                    state.GuiSettings.TXPath = WorkThread->PathToOpen;
                    state.Windows.emplace_back(std::make_unique<GUI::SenPatcherPatchTXWindow>(
                        state,
                        std::move(WorkThread->PathToOpen),
                        std::move(WorkThread->PatchDllPath),
                        std::move(WorkThread->PatchDllFile),
                        std::move(WorkThread->PatchDllInfo)));
                }
                WorkThread.reset();
                PendingWindowRequest = PendingWindowType::None;
            }
            break;
        }
    }
}
} // namespace SenTools::GUI
