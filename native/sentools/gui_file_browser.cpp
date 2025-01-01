#include "gui_file_browser.h"

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "imgui.h"

#include "gui_state.h"
#include "util/file.h"
#include "util/text.h"

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
    std::array<char, 1024> Filename;
    std::string Filter;
    bool PromptForOverwrite;
    bool Multiselect;

    std::optional<std::vector<FileEntry>> FilesInCurrentDirectory;
    ImGuiSelectionBasicStorage SelectionStorage;

    std::vector<std::string> SelectedPaths;
};

FileBrowser::FileBrowser() : PImpl(std::make_unique<Impl>()) {
    Reset(FileBrowserMode::OpenExistingFile, "", "", "All files (*)|*", false, false);
}

FileBrowser::~FileBrowser() = default;

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialPath,
                        std::string_view filter,
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

    Reset(mode, initialDirectory, suggestedFilename, filter, promptForOverwrite, multiselect);
}

void FileBrowser::Reset(FileBrowserMode mode,
                        std::string_view initialDirectory,
                        std::string_view suggestedFilename,
                        std::string_view filter,
                        bool promptForOverwrite,
                        bool multiselect) {
    PImpl->Mode = mode;

    if (initialDirectory.empty()) {
        PImpl->CurrentDirectory.reset();
    } else {
        PImpl->CurrentDirectory = HyoutaUtils::IO::FilesystemPathFromUtf8(initialDirectory);
    }
    HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(PImpl->Filename, suggestedFilename, true);
    PImpl->Filter = std::string(filter);
    PImpl->FilesInCurrentDirectory.reset();
    PImpl->SelectionStorage.Clear();
    PImpl->PromptForOverwrite = promptForOverwrite;
    PImpl->Multiselect = multiselect;
}

FileBrowserResult FileBrowser::RenderFrame(GuiState& state) {
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

const std::string& FileBrowser::GetSelectedPath() const {
    if (PImpl->SelectedPaths.empty()) {
        return "";
    }
    return PImpl->SelectedPaths[0];
}

const std::vector<std::string>& FileBrowser::GetSelectedPaths() const {
    return PImpl->SelectedPaths;
}
} // namespace SenTools::GUI
