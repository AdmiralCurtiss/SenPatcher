#include "system.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace HyoutaUtils::Sys {
#ifdef BUILD_FOR_WINDOWS
static std::optional<std::string> PostProcessExeDir(std::wstring_view exePath) {
    size_t lastPathSep = exePath.find_last_of(L"\\/");
    if (lastPathSep == std::wstring_view::npos) {
        return std::nullopt;
    }
    std::wstring_view dir = exePath.substr(0, lastPathSep);
    return HyoutaUtils::TextUtils::WStringToUtf8(dir.data(), dir.size());
}
#endif

std::optional<std::string> GetCurrentExecutableDirectory() noexcept {
#ifdef BUILD_FOR_WINDOWS
    // start with a stack buffer, this should work most of the time
    std::array<wchar_t, 1024> stackBuffer;
    SetLastError(0);
    DWORD result =
        GetModuleFileNameW(nullptr, stackBuffer.data(), static_cast<DWORD>(stackBuffer.size()));
    if (result == 0) {
        return std::nullopt;
    }
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return PostProcessExeDir(std::wstring_view(stackBuffer.data(), result));
    }

    // buffer was too small, keep retrying with larger ones or until we fail...
    std::wstring heapBuffer;
    heapBuffer.resize(stackBuffer.size() * 2);
    while (true) {
        SetLastError(0);
        DWORD result =
            GetModuleFileNameW(nullptr, heapBuffer.data(), static_cast<DWORD>(heapBuffer.size()));
        if (result == 0) {
            break;
        }
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return PostProcessExeDir(std::wstring_view(heapBuffer.data(), result));
        }
        if (heapBuffer.size() >= 0x1000'0000u) {
            // sanity break in case something goes really wrong...
            break;
        }
        heapBuffer.resize(heapBuffer.size() * 2);
    }
    return std::nullopt;
#else
    // TODO: Is there a reliable way to do this on Linux?
    return std::nullopt;
#endif
}
} // namespace HyoutaUtils::Sys
