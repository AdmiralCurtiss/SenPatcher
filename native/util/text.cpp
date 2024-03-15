#include "text.h"

#include <array>
#include <charconv>
#include <string>
#include <string_view>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace HyoutaUtils::TextUtils {
std::string Utf16ToUtf8(const char16_t* data, size_t length) {
    if (length > INT32_MAX) {
        throw "string too long";
    }

    const auto requiredBytes = WideCharToMultiByte(CP_UTF8,
                                                   0,
                                                   reinterpret_cast<const wchar_t*>(data),
                                                   static_cast<int>(length),
                                                   nullptr,
                                                   0,
                                                   nullptr,
                                                   nullptr);
    if (requiredBytes <= 0) {
        throw "string conversion failed";
    }

    std::string result;
    result.resize(requiredBytes);
    const auto convertedBytes = WideCharToMultiByte(CP_UTF8,
                                                    0,
                                                    reinterpret_cast<const wchar_t*>(data),
                                                    static_cast<int>(length),
                                                    result.data(),
                                                    result.size(),
                                                    nullptr,
                                                    nullptr);
    if (convertedBytes != requiredBytes) {
        throw "string conversion failed";
    }

    return result;
}

std::u16string Utf8ToUtf16(const char* data, size_t length) {
    if (length > INT32_MAX) {
        throw "string too long";
    }

    const auto requiredBytes = MultiByteToWideChar(CP_UTF8, 0, data, length, nullptr, 0);
    if (requiredBytes <= 0) {
        throw "string conversion failed";
    }

    std::u16string utf16;
    utf16.resize(requiredBytes);
    const auto convertedBytes =
        MultiByteToWideChar(CP_UTF8, 0, data, length, (wchar_t*)utf16.data(), utf16.size());
    if (convertedBytes != requiredBytes) {
        throw "string conversion failed";
    }

    return utf16;
}

std::string ShiftJisToUtf8(const char* data, size_t length) {
    if (length > INT32_MAX) {
        throw "string too long";
    }

    const auto requiredBytes = MultiByteToWideChar(932, 0, data, length, nullptr, 0);
    if (requiredBytes <= 0) {
        throw "string conversion failed";
    }

    std::u16string utf16;
    utf16.resize(requiredBytes);
    const auto convertedBytes =
        MultiByteToWideChar(932, 0, data, length, (wchar_t*)utf16.data(), utf16.size());
    if (convertedBytes != requiredBytes) {
        throw "string conversion failed";
    }

    return Utf16ToUtf8(utf16.data(), utf16.size());
}

std::string Replace(std::string_view input, std::string_view search, std::string_view replacement) {
    if (search.empty()) {
        return std::string(input);
    }

    std::string s;
    std::string_view rest = input;
    while (!rest.empty()) {
        if (rest.starts_with(search)) {
            s += replacement;
            rest.remove_prefix(search.size());
        } else {
            s += rest.front();
            rest.remove_prefix(1);
        }
    }
    return s;
}

std::string InsertSubstring(std::string_view a,
                            size_t offseta,
                            std::string_view b,
                            size_t offsetb,
                            size_t lengthb) {
    std::string s;
    s += a.substr(0, offseta);
    s += b.substr(offsetb, lengthb);
    s += a.substr(offseta);
    return s;
}

std::string ReplaceSubstring(std::string_view a,
                             size_t offseta,
                             size_t lengtha,
                             std::string_view b,
                             size_t offsetb,
                             size_t lengthb) {
    std::string s;
    s += a.substr(0, offseta);
    s += b.substr(offsetb, lengthb);
    s += a.substr(offseta + lengtha);
    return s;
}

std::string Remove(std::string_view input, size_t offset, size_t length) {
    std::string s;
    s += input.substr(0, offset);
    s += input.substr(offset + length);
    return s;
}

std::string Insert(std::string_view input, size_t offset, std::string_view insert) {
    std::string s;
    s += input.substr(0, offset);
    s += insert;
    s += input.substr(offset);
    return s;
}

std::string UInt32ToString(uint32_t value) {
    std::array<char, 16> str;
    auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), value, 10);
    if (ec == std::errc::value_too_large) {
        return "";
    }
    return std::string(std::string_view(str.data(), ptr));
}

std::string AdjustNewlinesToTwoSpaces(std::string desc) {
    size_t idx = 0;
    std::string s = std::move(desc);
    while (true) {
        size_t nidx = s.find_first_of('\n', idx);
        if (nidx == std::string::npos) {
            break;
        }

        size_t spaces = 0;
        size_t i = nidx + 1;
        while (i < s.size()) {
            if (s[i] == ' ') {
                ++i;
                ++spaces;
            } else {
                break;
            }
        }

        if (spaces != 2) {
            if (spaces < 2) {
                s = InsertSubstring(s, nidx + 1, "  ", 0, 2 - spaces);
            } else {
                s = Remove(s, nidx + 1, spaces - 2);
            }
        }

        idx = nidx + 1;
    }
    return s;
}

static bool IsWhitespace(char c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

std::string_view Trim(std::string_view sv) {
    while (!sv.empty() && IsWhitespace(sv.front())) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && IsWhitespace(sv.back())) {
        sv.remove_suffix(1);
    }
    return sv;
}

bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        const char cl = (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i];
        const char cr = (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i];
        if (cl != cr) {
            return false;
        }
    }
    return true;
}
} // namespace HyoutaUtils::TextUtils
