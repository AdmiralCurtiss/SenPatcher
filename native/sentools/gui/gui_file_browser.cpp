#include "gui_file_browser.h"

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "imgui.h"

#include "gui_state.h"
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
enum class FileEntryType : int {
    GoUpDirectory = -1,
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
    std::string DefaultExtension;
    bool PromptForOverwrite;
    bool Multiselect;
    bool UseNativeDialogIfAvailable;

    std::optional<std::vector<FileEntry>> FilesInCurrentDirectory;
    ImGuiSelectionBasicStorage SelectionStorage;

    std::vector<std::string> SelectedPaths;
};

FileBrowser::FileBrowser() : PImpl(std::make_unique<Impl>()) {
    Reset(FileBrowserMode::OpenExistingFile, "", "", {}, "", false, false);
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialPath,
                        std::vector<FileFilter> filter,
                        std::string_view defaultExtension,
                        bool promptForOverwrite,
                        bool multiselect) {
    size_t lastPathSep = initialPath.find_last_of(
#ifdef BUILD_FOR_WINDOWS
        "\\/"
#else
        '/'
#endif
    );

    std::string_view initialDirectory;
    std::string_view suggestedFilename;
    if (lastPathSep != std::string_view::npos) {
        initialDirectory = initialPath.substr(0, lastPathSep);
        suggestedFilename = initialPath.substr(lastPathSep + 1);
    } else {
        suggestedFilename = initialPath;
    }

    Reset(mode,
          initialDirectory,
          suggestedFilename,
          std::move(filter),
          defaultExtension,
          promptForOverwrite,
          multiselect);
}

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialDirectory,
                        std::string_view suggestedFilename,
                        std::vector<FileFilter> filter,
                        std::string_view defaultExtension,
                        bool promptForOverwrite,
                        bool multiselect) {
    PImpl->Mode = mode;

    if (initialDirectory.empty()) {
        PImpl->CurrentDirectory.reset();
    } else {
        PImpl->CurrentDirectory = HyoutaUtils::IO::FilesystemPathFromUtf8(initialDirectory);
    }
    PImpl->Filename = std::string(suggestedFilename);
    PImpl->Filters = std::move(filter);
    PImpl->DefaultExtension = std::string(defaultExtension);
    PImpl->FilesInCurrentDirectory.reset();
    PImpl->SelectionStorage.Clear();
    PImpl->PromptForOverwrite = promptForOverwrite;
    PImpl->Multiselect = multiselect;
    PImpl->UseNativeDialogIfAvailable = true;
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
        PImpl->CurrentDirectory.emplace(u8"C:\\");
#else
        PImpl->CurrentDirectory.emplace(u8"/");
#endif
    }

    if (!PImpl->FilesInCurrentDirectory) {
        // fill files
        PImpl->SelectionStorage.Clear();
        PImpl->FilesInCurrentDirectory.emplace();

        if (PImpl->CurrentDirectory->has_parent_path()) {
            auto& upDir = PImpl->FilesInCurrentDirectory->emplace_back();
            upDir.Filename = "..";
            upDir.Filesize = 0;
            upDir.Type = FileEntryType::GoUpDirectory;
        }

        std::error_code ec;
        std::filesystem::directory_iterator iterator(*PImpl->CurrentDirectory, ec);
        if (ec) {
            // not sure what we do here?
            return FileBrowserResult::Canceled;
        }
        for (auto const& entry : iterator) {
            auto& e = PImpl->FilesInCurrentDirectory->emplace_back();
            e.Filename = HyoutaUtils::IO::FilesystemPathToUtf8(entry.path().filename());
            e.Type = entry.is_directory(ec) ? FileEntryType::Directory : FileEntryType::File;
            e.Filesize = (e.Type == FileEntryType::File)
                             ? static_cast<uint64_t>(entry.file_size(ec))
                             : static_cast<uint64_t>(0);
        }

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
    }

    bool double_clicked = false;
    const float items_height = ImGui::GetTextLineHeightWithSpacing();
    ImGui::SetNextWindowContentSize(
        ImVec2(0.0f, PImpl->FilesInCurrentDirectory->size() * items_height));
    if (ImGui::BeginChild("##Basket",
                          ImVec2(-FLT_MIN, -(ImGui::GetFontSize() * 2)),
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
                ImGui::Selectable(
                    entry.Filename.c_str(), item_is_selected, ImGuiSelectableFlags_SpanAllColumns);

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    double_clicked = true;
                }

                ImGui::TableNextColumn();
                std::string_view type;
                if (entry.Type == FileEntryType::Directory) {
                    type = "Directory";
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


    if (ImGui::Button(PImpl->Mode == FileBrowserMode::OpenExistingFile ? "Open" : "Save")
        || double_clicked) {
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
                        PImpl->CurrentDirectory = PImpl->CurrentDirectory->parent_path();
                        PImpl->FilesInCurrentDirectory.reset();
                    } else if (entry.Type == FileEntryType::Directory) {
                        // go down once
                        PImpl->CurrentDirectory =
                            (*PImpl->CurrentDirectory)
                            / std::u8string_view((const char8_t*)entry.Filename.data(),
                                                 entry.Filename.size());
                        PImpl->FilesInCurrentDirectory.reset();
                    } else {
                        // selected a file
                        // TODO: ask for overwrite confirmation in save case
                        PImpl->SelectedPaths.clear();
                        PImpl->SelectedPaths.push_back(HyoutaUtils::IO::FilesystemPathToUtf8(
                            (*PImpl->CurrentDirectory)
                            / std::u8string_view((const char8_t*)entry.Filename.data(),
                                                 entry.Filename.size())));
                        return FileBrowserResult::FileSelected;
                    }
                }
            }
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
        return FileBrowserResult::Canceled;
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