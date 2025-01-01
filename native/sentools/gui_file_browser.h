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

struct FileBrowser {
    FileBrowser();
    FileBrowser(const FileBrowser& other) = delete;
    FileBrowser(FileBrowser&& other) = delete;
    FileBrowser& operator=(const FileBrowser& other) = delete;
    FileBrowser& operator=(FileBrowser&& other) = delete;
    ~FileBrowser();

    void Reset(FileBrowserMode mode,
               std::string_view initialPath,
               std::string_view filter,
               bool promptForOverwrite,
               bool multiselect);

    void Reset(FileBrowserMode mode,
               std::string_view initialDirectory,
               std::string_view suggestedFilename,
               std::string_view filter,
               bool promptForOverwrite,
               bool multiselect);

    FileBrowserResult RenderFrame(GuiState& state);

    const std::string& GetSelectedPath() const;
    const std::vector<std::string>& GetSelectedPaths() const;

private:
    struct Impl;
    std::unique_ptr<Impl> PImpl;
};
} // namespace SenTools::GUI
