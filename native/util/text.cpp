#include "text.h"

#include <array>
#include <charconv>
#include <string>
#include <string_view>
#include <vector>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace HyoutaUtils::TextUtils {
#ifdef _MSC_VER
static std::string WStringToCodepage(const wchar_t* data, size_t length, UINT codepage) {
    if (length == 0) {
        return std::string();
    }

    if (length > INT32_MAX) {
        throw "string too long";
    }

    const auto requiredBytes = WideCharToMultiByte(
        codepage, 0, data, static_cast<int>(length), nullptr, 0, nullptr, nullptr);
    if (requiredBytes <= 0) {
        throw "string conversion failed";
    }

    std::string result;
    result.resize(requiredBytes);
    const auto convertedBytes = WideCharToMultiByte(codepage,
                                                    0,
                                                    data,
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

template<typename T>
static T CodepageToString16(const char* data, size_t length, UINT codepage) {
    static_assert(sizeof(typename T::value_type) == 2);

    if (length == 0) {
        return T();
    }

    if (length > INT32_MAX) {
        throw "string too long";
    }

    const auto requiredBytes = MultiByteToWideChar(codepage, 0, data, length, nullptr, 0);
    if (requiredBytes <= 0) {
        throw "string conversion failed";
    }

    T wstr;
    wstr.resize(requiredBytes);
    const auto convertedBytes =
        MultiByteToWideChar(codepage, 0, data, length, (wchar_t*)wstr.data(), wstr.size());
    if (convertedBytes != requiredBytes) {
        throw "string conversion failed";
    }

    return wstr;
}
#endif

std::string Utf16ToUtf8(const char16_t* data, size_t length) {
#ifdef _MSC_VER
    return WStringToCodepage(reinterpret_cast<const wchar_t*>(data), length, CP_UTF8);
#else
    throw "not implemented";
#endif
}

std::u16string Utf8ToUtf16(const char* data, size_t length) {
#ifdef _MSC_VER
    return CodepageToString16<std::u16string>(data, length, CP_UTF8);
#else
    throw "not implemented";
#endif
}

std::string Utf16ToShiftJis(const char16_t* data, size_t length) {
#ifdef _MSC_VER
    return WStringToCodepage(reinterpret_cast<const wchar_t*>(data), length, 932);
#else
    throw "not implemented";
#endif
}

std::u16string ShiftJisToUtf16(const char* data, size_t length) {
#ifdef _MSC_VER
    return CodepageToString16<std::u16string>(data, length, 932);
#else
    throw "not implemented";
#endif
}

#ifdef _MSC_VER
std::string WStringToUtf8(const wchar_t* data, size_t length) {
    return WStringToCodepage(data, length, CP_UTF8);
}

std::wstring Utf8ToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, CP_UTF8);
}

std::string WStringToShiftJis(const wchar_t* data, size_t length) {
    return WStringToCodepage(data, length, 932);
}

std::wstring ShiftJisToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, 932);
}
#endif

std::string ShiftJisToUtf8(const char* data, size_t length) {
#ifdef _MSC_VER
    auto wstr = ShiftJisToWString(data, length);
    return WStringToUtf8(wstr.data(), wstr.size());
#else
    auto utf16 = ShiftJisToUtf16(data, length);
    return Utf16ToUtf8(utf16.data(), utf16.size());
#endif
}

std::string Utf8ToShiftJis(const char* data, size_t length) {
#ifdef _MSC_VER
    auto wstr = Utf8ToWString(data, length);
    return WStringToShiftJis(wstr.data(), wstr.size());
#else
    auto utf16 = Utf8ToUtf16(data, length);
    return Utf16ToShiftJis(utf16.data(), utf16.size());
#endif
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

std::vector<std::string_view> Split(std::string_view sv, std::string_view splitPattern) {
    std::vector<std::string_view> result;
    std::string_view rest = sv;
    while (true) {
        size_t offset = rest.find(splitPattern);
        if (offset == std::string_view::npos) {
            result.push_back(rest);
            return result;
        } else {
            result.push_back(rest.substr(0, offset));
            rest = rest.substr(offset + splitPattern.size());
        }
    }
}

std::string Join(const std::vector<std::string_view>& svs, std::string_view joinPattern) {
    std::string result;
    if (!svs.empty()) {
        result.append(svs[0]);
        for (size_t i = 1; i < svs.size(); ++i) {
            result.append(joinPattern);
            result.append(svs[i]);
        }
    }
    return result;
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
