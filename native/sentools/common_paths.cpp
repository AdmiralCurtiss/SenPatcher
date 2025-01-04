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
} // namespace SenTools::CommonPaths
