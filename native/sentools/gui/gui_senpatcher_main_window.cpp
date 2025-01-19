#include "gui_senpatcher_main_window.h"

#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "imgui.h"

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
#include "sen1/system_data.h"
#include "sen2/system_data.h"
#include "senpatcher_version.h"
#include "sentools/common_paths.h"
#include "sentools/old_senpatcher_unpatch.h"
#include "sentools/senpatcher_dll_loader.h"
#include "util/file.h"
#include "util/hash/sha1.h"
#include "util/scope.h"

namespace SenTools::GUI {
static void TextUnformatted(std::string_view sv) {
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}
static void TextUnformattedRightAlign(std::string_view sv) {
    float width = ImGui::CalcTextSize(sv.data(), sv.data() + sv.size()).x;
    ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetCursorPosX() + width));
    ImGui::TextUnformatted(sv.data(), sv.data() + sv.size());
}

struct SenPatcherMainWindow::WorkThreadState {
    std::atomic_bool CancelAll = false;
    std::atomic_bool IsDone = false;
    std::atomic_bool Success = false;

    enum class UserInputRequestType {
        None,
        ErrorMessage,
        YesNoQuestion,
    };
    enum class UserInputReplyType {
        None,
        OK,
        Yes,
        No,
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
};

SenPatcherMainWindow::SenPatcherMainWindow() = default;
SenPatcherMainWindow::~SenPatcherMainWindow() = default;

bool SenPatcherMainWindow::RenderFrame(GuiState& state) {
    bool visible = ImGui::Begin("SenPatcher", nullptr, ImGuiWindowFlags_MenuBar);
    const auto windowScope = HyoutaUtils::MakeScopeGuard([&]() { ImGui::End(); });
    if (visible) {
        RenderContents(state);
    }
    if (HasPendingWindowRequest()) {
        HandlePendingWindowRequest(state);
    }
    return true;
}

bool SenPatcherMainWindow::HasPendingWindowRequest() const {
    return PendingWindowRequest != PendingWindowType::None;
}

void SenPatcherMainWindow::RenderContents(GuiState& state) {
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
        ImGui::EndMenuBar();
    }

    TextUnformattedRightAlign("SenPatcher " SENPATCHER_VERSION);

    ImGui::Spacing();

    auto disabledScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { ImGui::EndDisabled(); });
    if (HasPendingWindowRequest()) {
        ImGui::BeginDisabled();
    } else {
        disabledScope.Dispose();
    }

    TextUnformatted("Trails of Cold Steel: (XSEED PC release version 1.6)");
    if (ImGui::Button("Patch game##1", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS1 game directory root", "Sen1Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathCS1()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select CS1 game directory root (Sen1Launcher.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    if (ImGui::Button("Edit System Save Data##1", ImVec2(-1.0f, 0.0f))
        && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS1 System Data file", "save511.dat"});
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
                          false);
        ImGui::OpenPopup("Select CS1 System Data file (save511.dat)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    TextUnformatted("Trails of Cold Steel II: (XSEED PC release version 1.4, 1.4.1, or 1.4.2)");
    if (ImGui::Button("Patch game##2", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS2 game directory root", "Sen2Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathCS2()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select CS2 game directory root (Sen2Launcher.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    if (ImGui::Button("Edit System Save Data##2", ImVec2(-1.0f, 0.0f))
        && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS2 System Data file", "save255.dat"});
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
                          false);
        ImGui::OpenPopup("Select CS2 System Data file (save255.dat)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    TextUnformatted("Trails of Cold Steel III: (NISA PC release version 1.07)");
    if (ImGui::Button("Patch game##3", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS3 game directory root", "Sen3Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathCS3()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select CS3 game directory root (Sen3Launcher.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    TextUnformatted("Trails of Cold Steel IV: (NISA PC release version 1.2.2)");
    if (ImGui::Button("Patch game##4", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"CS4 game directory root", "Sen4Launcher.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathCS4()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select CS4 game directory root (Sen4Launcher.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    TextUnformatted("Trails into Reverie: (NISA PC release version 1.1.5)");
    if (ImGui::Button("Patch game##5", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"Reverie game directory", "hnk.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathReverie()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select Reverie game directory (hnk.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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

    TextUnformatted("Tokyo Xanadu eX+: (Aksys PC release version 1.08)");
    if (ImGui::Button("Patch game##X", ImVec2(-1.0f, 0.0f)) && !HasPendingWindowRequest()) {
        std::vector<FileFilter> filters;
        filters.reserve(2);
        filters.push_back(FileFilter{"TX game directory root", "TokyoXanadu.exe"});
        filters.push_back(FileFilter{"All files (*.*)", "*"});
        GameBrowser.Reset(FileBrowserMode::OpenExistingFile,
                          "", // TODO: GamePaths.GetDefaultPathTX()
                          std::move(filters),
                          "exe",
                          false,
                          false);
        ImGui::OpenPopup("Select TX game directory root (TokyoXanadu.exe)");
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
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
                ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
                bool modal_open = true;
                if (ImGui::BeginPopupModal(
                        "Error", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    ImGui::TextUnformatted(WorkThread->UserInputRequestMessage.data(),
                                           WorkThread->UserInputRequestMessage.data()
                                               + WorkThread->UserInputRequestMessage.size());
                    ImGui::PopTextWrapPos();
                    if (ImGui::Button("OK", ImVec2(-1.0f, 0.0f))) {
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
                ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
                bool modal_open = true;
                if (ImGui::BeginPopupModal(
                        "Question", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
                    ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                    ImGui::TextUnformatted(WorkThread->UserInputRequestMessage.data(),
                                           WorkThread->UserInputRequestMessage.data()
                                               + WorkThread->UserInputRequestMessage.size());
                    ImGui::PopTextWrapPos();
                    if (ImGui::Button("Yes", ImVec2(-1.0f, 0.0f))) {
                        WorkThread->UserInputReply =
                            SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes;
                        ImGui::CloseCurrentPopup();
                        modal_open = false;
                    }
                    if (ImGui::Button("No", ImVec2(-1.0f, 0.0f))) {
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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        HyoutaUtils::IO::File f(std::string_view(selectedPath),
                                                HyoutaUtils::IO::OpenMode::Read);
                        if (!f.IsOpen()) {
                            threadState->ShowError("Could not open selected file.");
                            return;
                        }
                        const auto size = f.GetLength();
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 549888
                              || HyoutaUtils::Hash::CalculateSHA1FromFile(f)
                                     != HyoutaUtils::Hash::SHA1FromHexString(
                                         "8dde2b39f128179a0beb3301cfd56a98c0f98a55"))) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be Sen1Launcher.exe of "
                                    "version 1.6. Correct patching behavior cannot be guaranteed. "
                                    "Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);

                        // TODO: Check/fix filename encoding issues.

                        threadState->DoUnpatchWithUserConfirmation(path, 1);

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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        HyoutaUtils::IO::File f(std::string_view(selectedPath),
                                                HyoutaUtils::IO::OpenMode::Read);
                        if (!f.IsOpen()) {
                            threadState->ShowError("Could not open selected file.");
                            return;
                        }
                        const auto size = f.GetLength();
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 572928
                              || HyoutaUtils::Hash::CalculateSHA1FromFile(f)
                                     != HyoutaUtils::Hash::SHA1FromHexString(
                                         "81024410cc1fd1b462c600e0378714bd7704b202"))) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be Sen2Launcher.exe of "
                                    "version 1.4/1.4.1/1.4.2. Correct patching behavior cannot be "
                                    "guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);

                        // TODO: Check/fix filename encoding issues.

                        // TODO: Check for Humble v1.4 and handle.

                        threadState->DoUnpatchWithUserConfirmation(path, 2);

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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        HyoutaUtils::IO::File f(std::string_view(selectedPath),
                                                HyoutaUtils::IO::OpenMode::Read);
                        if (!f.IsOpen()) {
                            threadState->ShowError("Could not open selected file.");
                            return;
                        }
                        const auto size = f.GetLength();
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 569856
                              || HyoutaUtils::Hash::CalculateSHA1FromFile(f)
                                     != HyoutaUtils::Hash::SHA1FromHexString(
                                         "21de3b088a5ddad7ed1fdb8e40061497c248ca65"))) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be Sen3Launcher.exe of "
                                    "version 1.07. Correct patching behavior cannot be guaranteed. "
                                    "Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);

                        threadState->DoUnpatchWithUserConfirmation(path, 3);

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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        HyoutaUtils::IO::File f(std::string_view(selectedPath),
                                                HyoutaUtils::IO::OpenMode::Read);
                        if (!f.IsOpen()) {
                            threadState->ShowError("Could not open selected file.");
                            return;
                        }
                        const auto size = f.GetLength();
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 605184
                              || HyoutaUtils::Hash::CalculateSHA1FromFile(f)
                                     != HyoutaUtils::Hash::SHA1FromHexString(
                                         "5f480136aa4c3b53add422bf75b63350fa58d202"))) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be Sen4Launcher.exe of "
                                    "version 1.2.2. Correct patching behavior cannot be "
                                    "guaranteed. Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);

                        threadState->DoUnpatchWithUserConfirmation(path, 4);

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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        auto size = HyoutaUtils::IO::GetFilesize(std::string_view(selectedPath));
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 15807384)) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be hnk.exe of version 1.1.5. "
                                    "Correct patching behavior cannot be guaranteed. Proceed "
                                    "anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(
                                             HyoutaUtils::IO::SplitPath(
                                                 HyoutaUtils::IO::SplitPath(selectedPath).Directory)
                                                 .Directory)
                                             .Directory);

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
                        auto currentExeDir = HyoutaUtils::IO::GetCurrentExecutableDirectory();
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

                        auto size = HyoutaUtils::IO::GetFilesize(std::string_view(selectedPath));
                        if (!size) {
                            threadState->ShowError("Could not access selected file.");
                            return;
                        }
                        if (!(*size == 7373456 || *size == 7232000 || *size == 7225344)) {
                            if (threadState->ShowYesNoQuestion(
                                    "Selected file does not appear to be TokyoXanadu.exe of "
                                    "version 1.08. Correct patching behavior cannot be guaranteed. "
                                    "Proceed anyway?")
                                != SenPatcherMainWindow::WorkThreadState::UserInputReplyType::Yes) {
                                return;
                            }
                        }

                        std::string path(HyoutaUtils::IO::SplitPath(selectedPath).Directory);

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
