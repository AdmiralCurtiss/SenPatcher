#include "gui_file_browser.h"

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "gui_state.h"
#include "sentools_imgui_utils.h"
#include "util/file.h"
#include "util/scope.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shobjidl.h>
#endif

namespace SenTools::GUI {
namespace {
static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

enum class FileEntryType : int {
    GoUpDirectory = -2,
    Drive = -1, // Windows only
    Directory = 0,
    File = 1,
};

struct FileEntry {
    std::string Filename;
    uint64_t Filesize;
    FileEntryType Type;
    int Id;
};
} // namespace

struct FileBrowser::Impl {
    FileBrowserMode Mode;
    std::optional<std::filesystem::path> CurrentDirectory;
    std::string Filename;
    std::vector<FileFilter> Filters;
    std::string CurrentFilter;
    std::string DefaultExtension;
    size_t CurrentFilterIndex;
    bool PromptForOverwrite;
    bool Multiselect;
    bool UseNativeDialogIfAvailable;
    bool LastDirectoryChangeByKeyboardOrController;

    std::optional<std::vector<FileEntry>> FilesInCurrentDirectory;
    ImGuiSelectionBasicStorage SelectionStorage;

    std::vector<std::string> SelectedPaths;
};

FileBrowser::FileBrowser() : PImpl(std::make_unique<Impl>()) {
    Reset(FileBrowserMode::OpenExistingFile, "", "", {}, "", false, false, true);
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialPath,
                        std::vector<FileFilter> filter,
                        std::string_view defaultExtension,
                        bool promptForOverwrite,
                        bool multiselect,
                        bool useCustomFileBrowser) {
    auto split = HyoutaUtils::IO::SplitPath(initialPath);
    Reset(mode,
          split.Directory,
          split.Filename,
          std::move(filter),
          defaultExtension,
          promptForOverwrite,
          multiselect,
          useCustomFileBrowser);
}

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialDirectory,
                        std::string_view suggestedFilename,
                        std::vector<FileFilter> filter,
                        std::string_view defaultExtension,
                        bool promptForOverwrite,
                        bool multiselect,
                        bool useCustomFileBrowser) {
    PImpl->Mode = mode;

    if (initialDirectory.empty()) {
        PImpl->CurrentDirectory.reset();
    } else {
        PImpl->CurrentDirectory = HyoutaUtils::IO::FilesystemPathFromUtf8(initialDirectory);
    }
    PImpl->Filename = std::string(suggestedFilename);
    PImpl->Filters = std::move(filter);
    if (!PImpl->Filters.empty()) {
        PImpl->CurrentFilter = PImpl->Filters[0].Filter;
        PImpl->CurrentFilterIndex = 0;
    } else {
        PImpl->CurrentFilterIndex = INVALID_INDEX;
    }
    PImpl->DefaultExtension = std::string(defaultExtension);
    PImpl->FilesInCurrentDirectory.reset();
    PImpl->SelectionStorage.Clear();
    PImpl->PromptForOverwrite = promptForOverwrite;
    PImpl->Multiselect = multiselect;
    PImpl->UseNativeDialogIfAvailable = !useCustomFileBrowser;
    PImpl->LastDirectoryChangeByKeyboardOrController = false;
}

static bool IsRoot(const std::filesystem::path& p) {
#ifdef BUILD_FOR_WINDOWS
    return p.empty();
#else
    return p == u8"/";
#endif
}

FileBrowserResult FileBrowser::RenderFrame(GuiState& state, std::string_view title) {
#ifdef BUILD_FOR_WINDOWS
    if (PImpl->UseNativeDialogIfAvailable) {
        FileBrowserResult result = FileBrowserResult::Canceled;
        std::thread nativeFileDialogThread([&]() -> void {
            // native Windows file dialog is COM, so we need to init that
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (FAILED(hr)) {
                return;
            }
            auto coInitializeGuard = HyoutaUtils::MakeScopeGuard([]() { CoUninitialize(); });

            // the rest of the windows dialog code here is pretty much just an adaptation of
            // https://learn.microsoft.com/en-us/windows/win32/shell/common-file-dialog
            IFileDialog* pfd = nullptr;
            if (PImpl->Mode == FileBrowserMode::OpenExistingFile) {
                IFileOpenDialog* pfod = nullptr;
                hr = CoCreateInstance(
                    CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfod));
                pfd = pfod;
            } else if (PImpl->Mode == FileBrowserMode::SaveNewFile) {
                IFileSaveDialog* pfsd = nullptr;
                hr = CoCreateInstance(
                    CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfsd));
                pfd = pfsd;
            } else {
                return;
            }
            if (FAILED(hr)) {
                return;
            }
            auto coCreateGuard = HyoutaUtils::MakeScopeGuard([&]() { pfd->Release(); });

            // TODO: pfd->SetClientGuid()?

            DWORD options;
            hr = pfd->GetOptions(&options);
            if (FAILED(hr)) {
                return;
            }

            // TODO: Expose FOS_PICKFOLDERS somehow?

            options &= ~FOS_STRICTFILETYPES;
            options |= FOS_FORCEFILESYSTEM;
            if (PImpl->PromptForOverwrite && PImpl->Mode == FileBrowserMode::SaveNewFile) {
                options |= FOS_OVERWRITEPROMPT;
            } else {
                options &= ~FOS_OVERWRITEPROMPT;
            }
            if (PImpl->Multiselect && PImpl->Mode == FileBrowserMode::OpenExistingFile) {
                options |= FOS_ALLOWMULTISELECT;
            } else {
                options &= ~FOS_ALLOWMULTISELECT;
            }
            hr = pfd->SetOptions(options);
            if (FAILED(hr)) {
                return;
            }

            // not sure if these need to stay in memory so better be safe...
            std::vector<std::wstring> filterStrings;
            std::vector<COMDLG_FILTERSPEC> filterSpec;
            if (!PImpl->Filters.empty()) {
                const size_t sz = PImpl->Filters.size();
                filterStrings.reserve(sz * 2u);
                filterSpec.reserve(sz);
                for (const FileFilter& f : PImpl->Filters) {
                    auto name = HyoutaUtils::TextUtils::Utf8ToWString(f.Name.data(), f.Name.size());
                    if (!name) {
                        return;
                    }
                    auto filter =
                        HyoutaUtils::TextUtils::Utf8ToWString(f.Filter.data(), f.Filter.size());
                    if (!filter) {
                        return;
                    }
                    filterStrings.push_back(std::move(*name));
                    filterStrings.push_back(std::move(*filter));
                }
                for (size_t i = 0; i < sz; ++i) {
                    auto& spec = filterSpec.emplace_back();
                    spec.pszName = filterStrings[i * 2].c_str();
                    spec.pszSpec = filterStrings[i * 2 + 1].c_str();
                }
                hr = pfd->SetFileTypes(static_cast<UINT>(sz), filterSpec.data());
                if (FAILED(hr)) {
                    return;
                }
            }

            std::wstring defaultExtension;
            if (!PImpl->DefaultExtension.empty()) {
                auto ext = HyoutaUtils::TextUtils::Utf8ToWString(PImpl->DefaultExtension.data(),
                                                                 PImpl->DefaultExtension.size());
                if (!ext) {
                    return;
                }
                defaultExtension = std::move(*ext);
                hr = pfd->SetDefaultExtension(defaultExtension.c_str());
                if (FAILED(hr)) {
                    return;
                }
            }

            IShellItem* defaultFolder = nullptr;
            auto defaultFolderGuard = HyoutaUtils::MakeScopeGuard([&]() {
                if (defaultFolder != nullptr) {
                    defaultFolder->Release();
                }
            });
            if (PImpl->CurrentDirectory) {
                hr = SHCreateItemFromParsingName(
                    PImpl->CurrentDirectory->c_str(), nullptr, IID_PPV_ARGS(&defaultFolder));
                if (SUCCEEDED(hr)) {
                    pfd->SetFolder(defaultFolder);
                } else {
                    defaultFolder = nullptr;
                }
            }

            std::wstring defaultName;
            if (!PImpl->Filename.empty()) {
                auto name = HyoutaUtils::TextUtils::Utf8ToWString(PImpl->Filename.data(),
                                                                  PImpl->Filename.size());
                if (!name) {
                    return;
                }
                defaultName = std::move(*name);
                hr = pfd->SetFileName(defaultName.c_str());
                if (FAILED(hr)) {
                    return;
                }
            }

            std::wstring windowTitle;
            if (!title.empty()) {
                auto name = HyoutaUtils::TextUtils::Utf8ToWString(title.data(), title.size());
                if (!name) {
                    return;
                }
                windowTitle = std::move(*name);
                hr = pfd->SetTitle(windowTitle.c_str());
                if (FAILED(hr)) {
                    return;
                }
            }

            hr = pfd->Show(nullptr);
            if (FAILED(hr)) {
                return;
            }

            if (PImpl->Multiselect && PImpl->Mode == FileBrowserMode::OpenExistingFile) {
                IShellItemArray* psiaResults;
                hr = static_cast<IFileOpenDialog*>(pfd)->GetResults(&psiaResults);
                if (FAILED(hr)) {
                    return;
                }
                auto psiaResultsGuard =
                    HyoutaUtils::MakeScopeGuard([&]() { psiaResults->Release(); });

                PImpl->SelectedPaths.clear();
                DWORD count = 0;
                hr = psiaResults->GetCount(&count);
                if (FAILED(hr)) {
                    return;
                }

                PImpl->SelectedPaths.reserve(count);
                for (DWORD i = 0; i < count; ++i) {
                    IShellItem* psiResult;
                    hr = psiaResults->GetItemAt(i, &psiResult);
                    if (FAILED(hr)) {
                        return;
                    }

                    PWSTR pszFilePath = nullptr;
                    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (FAILED(hr)) {
                        return;
                    }
                    auto pszFilePathGuard =
                        HyoutaUtils::MakeScopeGuard([&]() { CoTaskMemFree(pszFilePath); });

                    auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(
                        pszFilePath, static_cast<size_t>(lstrlenW(pszFilePath)));
                    if (!utf8) {
                        return;
                    }

                    PImpl->SelectedPaths.push_back(std::move(*utf8));
                }
                result = FileBrowserResult::FileSelected;
            } else {
                IShellItem* psiResult;
                hr = pfd->GetResult(&psiResult);
                if (FAILED(hr)) {
                    return;
                }
                auto psiResultGuard = HyoutaUtils::MakeScopeGuard([&]() { psiResult->Release(); });

                PWSTR pszFilePath = nullptr;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (FAILED(hr)) {
                    return;
                }
                auto pszFilePathGuard =
                    HyoutaUtils::MakeScopeGuard([&]() { CoTaskMemFree(pszFilePath); });

                auto utf8 = HyoutaUtils::TextUtils::WStringToUtf8(
                    pszFilePath, static_cast<size_t>(lstrlenW(pszFilePath)));
                if (!utf8) {
                    return;
                }

                PImpl->SelectedPaths.clear();
                PImpl->SelectedPaths.push_back(std::move(*utf8));
                result = FileBrowserResult::FileSelected;
            }
        });
        nativeFileDialogThread.join();
        return result;
    }
#endif

    if (!PImpl->CurrentDirectory) {
#ifdef BUILD_FOR_WINDOWS
        PImpl->CurrentDirectory.emplace(); // we treat an empty path as the root above the drives
#else
        PImpl->CurrentDirectory.emplace(u8"/");
#endif
    }

    if (!PImpl->FilesInCurrentDirectory) {
        // fill files
        PImpl->SelectionStorage.Clear();
        PImpl->FilesInCurrentDirectory.emplace();

        const bool isRoot = IsRoot(*PImpl->CurrentDirectory);
        if (!isRoot) {
            auto& upDir = PImpl->FilesInCurrentDirectory->emplace_back();
            upDir.Filename = "..";
            upDir.Filesize = 0;
            upDir.Type = FileEntryType::GoUpDirectory;
        }

#ifdef BUILD_FOR_WINDOWS
        if (isRoot) {
            // On Windows we need to special-case the root and list the drives.
            for (const std::string& drive : HyoutaUtils::IO::GetLogicalDrives()) {
                auto& e = PImpl->FilesInCurrentDirectory->emplace_back();
                e.Filename = drive;
                e.Type = FileEntryType::Drive;
                e.Filesize = static_cast<uint64_t>(0);
            }
        } else {
#endif
            std::error_code ec;
            std::filesystem::directory_iterator it(*PImpl->CurrentDirectory, ec);
            if (ec) {
                // not sure what we do here?
                return FileBrowserResult::Canceled;
            }
            while (it != std::filesystem::directory_iterator()) {
                const bool isDirectory = it->is_directory(ec);
                if (!ec) {
                    std::string filename =
                        HyoutaUtils::IO::FilesystemPathToUtf8(it->path().filename());
                    if (isDirectory || PImpl->CurrentFilter.empty()
                        || HyoutaUtils::TextUtils::CaseInsensitiveGlobMatches(
                            filename, PImpl->CurrentFilter)) {
                        auto& e = PImpl->FilesInCurrentDirectory->emplace_back();
                        e.Filename = std::move(filename);
                        e.Type = isDirectory ? FileEntryType::Directory : FileEntryType::File;
                        e.Filesize = (e.Type == FileEntryType::File)
                                         ? static_cast<uint64_t>(it->file_size(ec))
                                         : static_cast<uint64_t>(0);
                    }
                }
                it.increment(ec);
                if (ec) {
                    // not sure what we do here?
                    return FileBrowserResult::Canceled;
                }
            }
#ifdef BUILD_FOR_WINDOWS
        }
#endif

        std::stable_sort(PImpl->FilesInCurrentDirectory->begin(),
                         PImpl->FilesInCurrentDirectory->end(),
                         [](const FileEntry& lhs, const FileEntry& rhs) {
                             if (lhs.Type != rhs.Type) {
                                 return lhs.Type < rhs.Type;
                             }
                             return HyoutaUtils::TextUtils::CaseInsensitiveCompare(lhs.Filename,
                                                                                   rhs.Filename)
                                    < 0;
                         });

        for (size_t i = 0; i < PImpl->FilesInCurrentDirectory->size(); ++i) {
            (*PImpl->FilesInCurrentDirectory)[i].Id = static_cast<int>(i);
        }

        if (PImpl->FilesInCurrentDirectory->size() > 0) {
            PImpl->SelectionStorage.SetItemSelected((ImGuiID)0, true);

            if (PImpl->LastDirectoryChangeByKeyboardOrController) {
                PImpl->Filename = (*PImpl->FilesInCurrentDirectory)[0].Filename;
            }
        }

        PImpl->LastDirectoryChangeByKeyboardOrController = false;
    }

    bool double_clicked = false;
    bool enter_pressed = false;
    const float items_height = ImGui::GetTextLineHeightWithSpacing();
    ImGui::SetNextWindowContentSize(
        ImVec2(0.0f, PImpl->FilesInCurrentDirectory->size() * items_height));
    if (ImGui::BeginChild("##Basket",
                          ImVec2(-FLT_MIN, -(ImGui::GetFontSize() * 4.7f)),
                          ImGuiChildFlags_FrameStyle)) {
        ImGuiMultiSelectIO* ms_io =
            ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnEscape
                                        | (PImpl->Multiselect ? ImGuiMultiSelectFlags_None
                                                              : ImGuiMultiSelectFlags_SingleSelect),
                                    PImpl->SelectionStorage.Size,
                                    PImpl->FilesInCurrentDirectory->size());
        PImpl->SelectionStorage.ApplyRequests(ms_io);

        ImGui::BeginTable("##Split",
                          3,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings
                              | ImGuiTableFlags_NoPadOuterX);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.70f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.10f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 0.20f);

        ImGuiListClipper clipper;
        clipper.Begin(PImpl->FilesInCurrentDirectory->size());
        if (ms_io->RangeSrcItem != -1)
            clipper.IncludeItemByIndex(
                (int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

        while (clipper.Step()) {
            const int item_begin = clipper.DisplayStart;
            const int item_end = clipper.DisplayEnd;
            for (int n = item_begin; n < item_end; ++n) {
                ImGui::TableNextColumn();

                const auto& entry = (*PImpl->FilesInCurrentDirectory)[n];
                const int item_id = entry.Id;

                // IMPORTANT: for deletion refocus to work we need object ID to be stable,
                // aka not depend on their index in the list. Here we use our persistent item_id
                // instead of index to build a unique ID that will persist.
                // (If we used PushID(index) instead, focus wouldn't be restored correctly after
                // deletion).
                ImGui::PushID(item_id);

                // Submit item
                bool item_is_selected = PImpl->SelectionStorage.Contains((ImGuiID)n);
                ImGui::SetNextItemSelectionUserData(n);
                if (ImGui::Selectable(entry.Filename.c_str(),
                                      item_is_selected,
                                      ImGuiSelectableFlags_SpanAllColumns
                                          | ImGuiSelectableFlags_AllowDoubleClick)) {
                    PImpl->Filename = entry.Filename;

                    // double-click on item
                    if (ImGui::IsItemHovered()
                        && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        double_clicked = true;
                    }

                    // keyboard enter or controller confirm (is there a better way to do this?)
                    if (ImGui::IsItemFocused() && ImGui::IsItemActivated()
                        && !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        enter_pressed = true;
                    }
                }

                ImGui::TableNextColumn();
                std::string_view type;
                if (entry.Type == FileEntryType::Directory) {
                    type = "Directory";
                } else if (entry.Type == FileEntryType::Drive) {
                    type = "Drive";
                } else if (entry.Type == FileEntryType::File) {
                    type = "File";
                }
                ImGui::TextUnformatted(type.data(), type.data() + type.size());
                ImGui::TableNextColumn();
                if (entry.Type == FileEntryType::File) {
                    ImGui::Text("%" PRIu64 " bytes", entry.Filesize);
                }

                ImGui::PopID();
            }
        }

        ImGui::EndTable();

        // Apply multi-select requests
        ms_io = ImGui::EndMultiSelect();
        PImpl->SelectionStorage.ApplyRequests(ms_io);
    }
    ImGui::EndChild();

    static constexpr std::string_view filenameLabel = "File name";
    static constexpr std::string_view filetypeLabel = "File type";
    const float filenameLabelWidth =
        ImGui::CalcTextSize(filenameLabel.data(), filenameLabel.data() + filenameLabel.size()).x;
    const float filetypeLabelWidth =
        ImGui::CalcTextSize(filetypeLabel.data(), filetypeLabel.data() + filetypeLabel.size()).x;
    const float labelWidth = std::max(filenameLabelWidth, filetypeLabelWidth);

    ImGui::AlignTextToFramePadding();
    ImGuiUtils::TextUnformatted(filenameLabel);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (labelWidth - filenameLabelWidth));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##Filename", &PImpl->Filename, ImGuiInputTextFlags_ElideLeft);

    ImGui::AlignTextToFramePadding();
    ImGuiUtils::TextUnformatted(filetypeLabel);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (labelWidth - filetypeLabelWidth));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##Filter",
                          PImpl->CurrentFilterIndex < PImpl->Filters.size()
                              ? PImpl->Filters[PImpl->CurrentFilterIndex].Name.c_str()
                              : PImpl->CurrentFilter.c_str(),
                          ImGuiComboFlags_HeightRegular)) {
        for (size_t n = 0; n < PImpl->Filters.size(); ++n) {
            const bool is_selected = (PImpl->CurrentFilterIndex == n);
            if (ImGui::Selectable(PImpl->Filters[n].Name.c_str(), is_selected)) {
                PImpl->CurrentFilter = PImpl->Filters[n].Filter;
                PImpl->CurrentFilterIndex = n;
                PImpl->FilesInCurrentDirectory.reset();
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    static constexpr char openLabel[] = "Open";
    static constexpr char saveLabel[] = "Save";
    static constexpr char cancelLabel[] = "Cancel";
    const char* confirmLabel =
        PImpl->Mode == FileBrowserMode::OpenExistingFile ? openLabel : saveLabel;
    float confirmTextWidth = ImGui::CalcTextSize(confirmLabel, nullptr, true).x;
    float cancelTextWidth = ImGui::CalcTextSize(cancelLabel, nullptr, true).x;
    float framePadding = ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x
                         - (confirmTextWidth + cancelTextWidth + framePadding * 2.0f
                            + ImGui::GetStyle().ItemSpacing.x));

    bool button_clicked = ImGui::Button(confirmLabel);
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button(cancelLabel)) {
        return FileBrowserResult::Canceled;
    }

    if (button_clicked) {
#ifdef BUILD_FOR_WINDOWS
        // if we're in the top of a drive and want to go up, we need to go up to the drive selection
        if (PImpl->Filename == ".."
            && *PImpl->CurrentDirectory == PImpl->CurrentDirectory->root_path()) {
            PImpl->CurrentDirectory.emplace();
            PImpl->FilesInCurrentDirectory.reset();
            PImpl->Filename.clear();
        } else {
#endif
            bool isValidPath = false;
            auto path = HyoutaUtils::IO::FilesystemPathFromUtf8(PImpl->Filename);
            if (path.is_relative()) {
                path = (*PImpl->CurrentDirectory / path);
            }
            std::error_code ec{};
            path = std::filesystem::weakly_canonical(path, ec);
            if (!ec) {
                const auto fileStatus = std::filesystem::status(path, ec);
                if (fileStatus.type() != std::filesystem::file_type::none
                    && (PImpl->Mode == FileBrowserMode::SaveNewFile
                        || fileStatus.type() != std::filesystem::file_type::not_found)) {
                    isValidPath = true;
                    if (fileStatus.type() == std::filesystem::file_type::directory) {
                        // enter this directory
                        PImpl->CurrentDirectory = std::move(path);
                        PImpl->FilesInCurrentDirectory.reset();
                        PImpl->Filename.clear();
                    } else {
                        // treat this as the result
                        PImpl->SelectedPaths.clear();
                        PImpl->SelectedPaths.push_back(HyoutaUtils::IO::FilesystemPathToUtf8(path));
                        if (PImpl->Mode == FileBrowserMode::SaveNewFile
                            && fileStatus.type() != std::filesystem::file_type::not_found) {
                            // when saving and the file exists, ask if overwriting is okay
                            ImGui::OpenPopup("Overwrite?");
                        } else {
                            return FileBrowserResult::FileSelected;
                        }
                    }
                }
            }

            if (!isValidPath) {
                // if the user entered something like "*.txt", set the filter to that
                if (PImpl->Filename.find('*') != std::string::npos) {
                    PImpl->CurrentFilter = PImpl->Filename;
                    PImpl->CurrentFilterIndex = INVALID_INDEX;
                    PImpl->FilesInCurrentDirectory.reset();
                }
            }
#ifdef BUILD_FOR_WINDOWS
        }
#endif
    }

    if (double_clicked || enter_pressed) {
        void* it = nullptr;
        ImGuiID id;
        while (PImpl->SelectionStorage.GetNextSelectedItem(&it, &id)) {
            if (PImpl->Multiselect) {
                // TODO
            } else {
                auto entryIt = std::find_if(PImpl->FilesInCurrentDirectory->begin(),
                                            PImpl->FilesInCurrentDirectory->end(),
                                            [id](const FileEntry& e) { return e.Id == id; });
                if (entryIt != PImpl->FilesInCurrentDirectory->end()) {
                    const auto& entry = *entryIt;
                    if (entry.Type == FileEntryType::GoUpDirectory) {
                        // go up once
#ifdef BUILD_FOR_WINDOWS
                        // if we're in the top of a drive, we need to go up to the drive selection
                        if (*PImpl->CurrentDirectory == PImpl->CurrentDirectory->root_path()) {
                            PImpl->CurrentDirectory.emplace();
                        } else {
#endif
                            PImpl->CurrentDirectory = PImpl->CurrentDirectory->parent_path();
#ifdef BUILD_FOR_WINDOWS
                        }
#endif
                        PImpl->FilesInCurrentDirectory.reset();
                        PImpl->Filename.clear();
                        PImpl->LastDirectoryChangeByKeyboardOrController = enter_pressed;
                    } else if (entry.Type == FileEntryType::Directory) {
                        // go down once
                        PImpl->CurrentDirectory =
                            (*PImpl->CurrentDirectory)
                            / std::u8string_view((const char8_t*)entry.Filename.data(),
                                                 entry.Filename.size());
                        PImpl->FilesInCurrentDirectory.reset();
                        PImpl->Filename.clear();
                        PImpl->LastDirectoryChangeByKeyboardOrController = enter_pressed;
                    } else if (entry.Type == FileEntryType::Drive) {
                        // enter drive
                        PImpl->CurrentDirectory = std::u8string_view(
                            (const char8_t*)entry.Filename.data(), entry.Filename.size());
                        PImpl->FilesInCurrentDirectory.reset();
                        PImpl->Filename.clear();
                        PImpl->LastDirectoryChangeByKeyboardOrController = enter_pressed;
                    } else if (entry.Type == FileEntryType::File) {
                        // selected a file
                        PImpl->SelectedPaths.clear();
                        PImpl->SelectedPaths.push_back(HyoutaUtils::IO::FilesystemPathToUtf8(
                            (*PImpl->CurrentDirectory)
                            / std::u8string_view((const char8_t*)entry.Filename.data(),
                                                 entry.Filename.size())));
                        if (PImpl->Mode == FileBrowserMode::SaveNewFile) {
                            // when saving and the file exists, ask if overwriting is okay
                            ImGui::OpenPopup("Overwrite?");
                        } else {
                            return FileBrowserResult::FileSelected;
                        }
                    }
                }
            }
        }
    }

    bool modal_open = true;
    bool shouldOverwrite = false;
    ImGuiUtils::SetNextWindowSizeForStandardPopup();
    if (ImGui::BeginPopupModal("Overwrite?", &modal_open, ImGuiWindowFlags_NoSavedSettings)) {
        if (!PImpl->SelectedPaths.empty()) {
            if (PImpl->SelectedPaths.size() == 1) {
                ImGui::Text("Overwrite %s?", PImpl->SelectedPaths[0].c_str());
            } else {
                ImGuiUtils::TextUnformatted("Overwrite the selected files?");
            }
            if (ImGuiUtils::ButtonFullWidth("Overwrite")) {
                ImGui::CloseCurrentPopup();
                shouldOverwrite = true;
            }
            ImGui::SetItemDefaultFocus();
            if (ImGuiUtils::ButtonFullWidth("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (shouldOverwrite) {
        return FileBrowserResult::FileSelected;
    }

    return FileBrowserResult::None;
}

std::string_view FileBrowser::GetSelectedPath() const {
    if (PImpl->SelectedPaths.empty()) {
        return "";
    }
    return PImpl->SelectedPaths[0];
}

const std::vector<std::string>& FileBrowser::GetSelectedPaths() const {
    return PImpl->SelectedPaths;
}
} // namespace SenTools::GUI
