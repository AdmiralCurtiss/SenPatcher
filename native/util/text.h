#pragma once

#include <string>
#include <string_view>

namespace HyoutaUtils::TextUtils {
enum class GameTextEncoding {
    ASCII,
    ShiftJIS,
    UTF8,
    UTF16,
};

std::string Utf16ToUtf8(const char16_t* data, size_t length);
std::u16string Utf8ToUtf16(const char* data, size_t length);
std::string ShiftJisToUtf8(const char* data, size_t length);

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

bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs);
} // namespace HyoutaUtils::TextUtils
