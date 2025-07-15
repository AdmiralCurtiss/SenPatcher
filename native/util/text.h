#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace HyoutaUtils::TextUtils {
enum class GameTextEncoding {
    ASCII,
    ShiftJIS,
    UTF8,
    UTF16,
};

std::optional<std::string> Utf16ToUtf8(const char16_t* data, size_t length) noexcept;
std::optional<std::u16string> Utf8ToUtf16(const char* data, size_t length) noexcept;
std::optional<std::string> Utf16ToShiftJis(const char16_t* data, size_t length) noexcept;
std::optional<std::u16string> ShiftJisToUtf16(const char* data, size_t length) noexcept;

#ifdef BUILD_FOR_WINDOWS
// this is technically identical to the utf16 variants, but the compiler considers them separate
// types...
std::optional<std::string> WStringToUtf8(const wchar_t* data, size_t length) noexcept;
std::optional<std::wstring> Utf8ToWString(const char* data, size_t length) noexcept;
std::optional<std::string> WStringToShiftJis(const wchar_t* data, size_t length) noexcept;
std::optional<std::wstring> ShiftJisToWString(const char* data, size_t length) noexcept;
#endif

std::optional<std::string> ShiftJisToUtf8(const char* data, size_t length) noexcept;
std::optional<std::string> Utf8ToShiftJis(const char* data, size_t length) noexcept;

#ifdef BUILD_FOR_WINDOWS
std::optional<std::wstring> AnsiCodePageToWString(const char* data, size_t length) noexcept;
std::optional<std::string> AnsiCodePageToUtf8(const char* data, size_t length) noexcept;
std::optional<std::wstring> OemCodePageToWString(const char* data, size_t length) noexcept;
std::optional<std::string> OemCodePageToUtf8(const char* data, size_t length) noexcept;
#endif

std::string Replace(std::string_view input, std::string_view search, std::string_view replacement);
std::string InsertSubstring(std::string_view a,
                            size_t offseta,
                            std::string_view b,
                            size_t offsetb,
                            size_t lengthb);
std::string ReplaceSubstring(std::string_view a, size_t offset, size_t length, std::string_view b);
std::string ReplaceSubstring(std::string_view a,
                             size_t offseta,
                             size_t lengtha,
                             std::string_view b,
                             size_t offsetb,
                             size_t lengthb);
std::string MoveSubstring(std::string_view input,
                          size_t sourceLocation,
                          size_t targetLocation,
                          size_t length);
std::string Remove(std::string_view input, size_t offset, size_t length);
std::string Insert(std::string_view input, size_t offset, std::string_view insert);

std::string UInt32ToString(uint32_t value);

std::string_view Trim(std::string_view sv) noexcept;

std::vector<std::string_view> Split(std::string_view sv, std::string_view splitPattern);
std::string Join(const std::vector<std::string>& svs, std::string_view joinPattern);
std::string Join(const std::vector<std::string_view>& svs, std::string_view joinPattern);

// These only care about the ASCII A-Z <-> a-z, everything else is still handled case sensitive.
bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs) noexcept;
int CaseInsensitiveCompare(std::string_view lhs, std::string_view rhs) noexcept;
std::string ToLower(std::string_view sv);
std::string ToUpper(std::string_view sv);

bool GlobMatches(std::string_view string, std::string_view glob) noexcept;
bool CaseInsensitiveGlobMatches(std::string_view string, std::string_view glob) noexcept;

// Extracts a string from a fixed-length maybe-nullterminated buffer.
inline std::string_view StripToNull(std::string_view sv) noexcept {
    for (size_t i = 0; i < sv.size(); ++i) {
        if (sv[i] == '\0') {
            return sv.substr(0, i);
        }
    }
    return sv;
}
template<size_t length>
inline std::string_view StripToNull(const std::array<char, length>& arr) noexcept {
    return StripToNull(std::string_view(arr.data(), arr.size()));
}

inline void WriteToFixedLengthBuffer(char* buffer,
                                     size_t length,
                                     std::string_view source,
                                     bool mustBeNullTerminated) noexcept {
    if (buffer == nullptr || length == 0) {
        return;
    }

    std::memset(buffer, 0, length);
    const size_t l = (mustBeNullTerminated ? (length - 1) : length);
    std::memcpy(buffer, source.data(), l < source.size() ? l : source.size());
}
template<size_t length>
inline void WriteToFixedLengthBuffer(std::array<char, length>& target,
                                     std::string_view source,
                                     bool mustBeNullTerminated) noexcept {
    WriteToFixedLengthBuffer(target.data(), target.size(), source, mustBeNullTerminated);
}
} // namespace HyoutaUtils::TextUtils
