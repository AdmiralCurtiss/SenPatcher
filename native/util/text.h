#pragma once

#include <cstdint>
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

std::optional<std::string> Utf16ToUtf8(const char16_t* data, size_t length);
std::optional<std::u16string> Utf8ToUtf16(const char* data, size_t length);
std::optional<std::string> Utf16ToShiftJis(const char16_t* data, size_t length);
std::optional<std::u16string> ShiftJisToUtf16(const char* data, size_t length);

#ifdef BUILD_FOR_WINDOWS
// this is technically identical to the utf16 variants, but the compiler considers them separate
// types...
std::optional<std::string> WStringToUtf8(const wchar_t* data, size_t length);
std::optional<std::wstring> Utf8ToWString(const char* data, size_t length);
std::optional<std::string> WStringToShiftJis(const wchar_t* data, size_t length);
std::optional<std::wstring> ShiftJisToWString(const char* data, size_t length);
#endif

std::optional<std::string> ShiftJisToUtf8(const char* data, size_t length);
std::optional<std::string> Utf8ToShiftJis(const char* data, size_t length);

std::string Replace(std::string_view input, std::string_view search, std::string_view replacement);
std::string InsertSubstring(std::string_view a,
                            size_t offseta,
                            std::string_view b,
                            size_t offsetb,
                            size_t lengthb);
std::string ReplaceSubstring(std::string_view a,
                             size_t offseta,
                             size_t lengtha,
                             std::string_view b,
                             size_t offsetb,
                             size_t lengthb);
std::string Remove(std::string_view input, size_t offset, size_t length);
std::string Insert(std::string_view input, size_t offset, std::string_view insert);

std::string UInt32ToString(uint32_t value);

std::string AdjustNewlinesToTwoSpaces(std::string desc);

std::string_view Trim(std::string_view sv);

std::vector<std::string_view> Split(std::string_view sv, std::string_view splitPattern);
std::string Join(const std::vector<std::string_view>& svs, std::string_view joinPattern);

bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs);
} // namespace HyoutaUtils::TextUtils
