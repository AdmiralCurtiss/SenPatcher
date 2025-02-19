#include "system.h"

#include <array>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "util/text.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <stdlib.h>
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
        result =
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

std::optional<std::string> GetEnvironmentVar(const char* key) noexcept {
    if (!key) {
        return std::nullopt;
    }

#ifdef BUILD_FOR_WINDOWS
    auto wkey = HyoutaUtils::TextUtils::Utf8ToWString(key, strlen(key));
    if (!wkey) {
        return std::nullopt;
    }
    DWORD bufferSize = GetEnvironmentVariableW(wkey->c_str(), nullptr, 0);
    if (bufferSize == 0) {
        return std::nullopt;
    }
    std::array<wchar_t, 1024> stackBuffer;
    std::unique_ptr<wchar_t[]> heapBuffer;
    wchar_t* buffer;
    if (bufferSize <= stackBuffer.size()) {
        buffer = stackBuffer.data();
        bufferSize = static_cast<DWORD>(stackBuffer.size());
    } else {
        buffer = new (std::nothrow) wchar_t[bufferSize];
        if (!buffer) {
            return std::nullopt;
        }
        heapBuffer.reset(buffer);
    }
    const DWORD bufferSize2 = GetEnvironmentVariableW(wkey->c_str(), buffer, bufferSize);
    if (bufferSize2 > bufferSize) {
        // can this happen?
        return std::nullopt;
    }
    return HyoutaUtils::TextUtils::WStringToUtf8(buffer, bufferSize2);
#else
    auto* v = getenv(key);
    if (!v) {
        return std::nullopt;
    }
    return std::string(v);
#endif
}
} // namespace HyoutaUtils::Sys
