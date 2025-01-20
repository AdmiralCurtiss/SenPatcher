#include "common_paths.h"

#include <optional>
#include <string>

#include "util/scope.h"
#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <Shlobj_core.h>
#else
#include <stdlib.h>
#endif

namespace SenTools::CommonPaths {
std::optional<std::string> GetSavedGamesFolder() {
#ifdef BUILD_FOR_WINDOWS
    PWSTR path;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &path))) {
        return std::nullopt;
    }
    auto pathScope = HyoutaUtils::MakeScopeGuard([&path]() { CoTaskMemFree(path); });
    return HyoutaUtils::TextUtils::WStringToUtf8(path, static_cast<size_t>(lstrlenW(path)));
#else
    return std::nullopt;
#endif
}

std::optional<std::string> GetLocalAppDataFolder() {
#ifdef BUILD_FOR_WINDOWS
    PWSTR path;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
        return std::nullopt;
    }
    auto pathScope = HyoutaUtils::MakeScopeGuard([&path]() { CoTaskMemFree(path); });
    return HyoutaUtils::TextUtils::WStringToUtf8(path, static_cast<size_t>(lstrlenW(path)));
#else
    const char* xdg_state_home = getenv("XDG_STATE_HOME");
    if (xdg_state_home && *xdg_state_home != '\0') {
        return std::string(xdg_state_home);
    }
    const char* home = getenv("HOME");
    if (home && *home != '\0') {
        std::string result(home);
        if (!result.ends_with('/')) {
            result.push_back('/');
        }
        result.append(".local/state");
        return result;
    }
    return std::nullopt;
#endif
}

std::optional<std::string> GetLocalSenPatcherGuiSettingsFolder() {
    std::optional<std::string> result = GetLocalAppDataFolder();
    if (!result) {
        return std::nullopt;
    }
#ifdef BUILD_FOR_WINDOWS
    if (!(result->ends_with('\\') || result->ends_with('/'))) {
        result->push_back('\\');
    }
#else
    if (!result->ends_with('/')) {
        result->push_back('/');
    }
#endif
    result->append("SenPatcherGui");
    return result;
}
} // namespace SenTools::CommonPaths
