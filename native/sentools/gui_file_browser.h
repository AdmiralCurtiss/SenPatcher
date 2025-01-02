#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace SenTools {
struct GuiState;
}

namespace SenTools::GUI {
enum class FileBrowserMode {
    OpenExistingFile,
    SaveNewFile,
};
enum class FileBrowserResult {
    None,
    FileSelected,
    Canceled,
};
struct FileFilter {
    std::string Name;
    std::string Filter;
};

struct FileBrowser {
    FileBrowser();
    FileBrowser(const FileBrowser& other) = delete;
    FileBrowser(FileBrowser&& other) = delete;
    FileBrowser& operator=(const FileBrowser& other) = delete;
    FileBrowser& operator=(FileBrowser&& other) = delete;
    ~FileBrowser();

    void Reset(FileBrowserMode mode,
               std::string_view initialPath,
               std::vector<FileFilter> filter,
               std::string_view defaultExtension,
               bool promptForOverwrite,
               bool multiselect);

    void Reset(FileBrowserMode mode,
               std::string_view initialDirectory,
               std::string_view suggestedFilename,
               std::vector<FileFilter> filter,
               std::string_view defaultExtension,
               bool promptForOverwrite,
               bool multiselect);

    FileBrowserResult RenderFrame(GuiState& state, std::string_view title);

    std::string_view GetSelectedPath() const;
    const std::vector<std::string>& GetSelectedPaths() const;

private:
    struct Impl;
    std::unique_ptr<Impl> PImpl;
};
} // namespace SenTools::GUI
