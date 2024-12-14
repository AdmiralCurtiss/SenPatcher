#include "text.h"

#include <array>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <iconv.h>
#endif

namespace HyoutaUtils::TextUtils {
#ifdef BUILD_FOR_WINDOWS
static std::optional<std::string>
    WStringToCodepage(const wchar_t* data, size_t length, UINT codepage) {
    if (length == 0) {
        return std::string();
    }

    if (length > INT32_MAX) {
        return std::nullopt;
    }

    BOOL usedDefaultChar = FALSE;
    const DWORD flags = (codepage == CP_UTF8) ? WC_ERR_INVALID_CHARS : WC_NO_BEST_FIT_CHARS;
    const LPBOOL usedDefaultCharPtr = (codepage == CP_UTF8) ? nullptr : &usedDefaultChar;

    const auto requiredBytes = WideCharToMultiByte(
        codepage, flags, data, static_cast<int>(length), nullptr, 0, nullptr, usedDefaultCharPtr);
    if (requiredBytes <= 0 || usedDefaultChar != FALSE) {
        return std::nullopt;
    }

    std::string result;
    result.resize(requiredBytes);
    const auto convertedBytes = WideCharToMultiByte(codepage,
                                                    flags,
                                                    data,
                                                    static_cast<int>(length),
                                                    result.data(),
                                                    result.size(),
                                                    nullptr,
                                                    usedDefaultCharPtr);
    if (convertedBytes != requiredBytes || usedDefaultChar != FALSE) {
        return std::nullopt;
    }

    return result;
}

template<typename T>
static std::optional<T> CodepageToString16(const char* data, size_t length, UINT codepage) {
    static_assert(sizeof(typename T::value_type) == 2);

    if (length == 0) {
        return T();
    }

    if (length > INT32_MAX) {
        return std::nullopt;
    }

    const auto requiredBytes =
        MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, data, length, nullptr, 0);
    if (requiredBytes <= 0) {
        return std::nullopt;
    }

    T wstr;
    wstr.resize(requiredBytes);
    const auto convertedBytes = MultiByteToWideChar(
        codepage, MB_ERR_INVALID_CHARS, data, length, (wchar_t*)wstr.data(), wstr.size());
    if (convertedBytes != requiredBytes) {
        return std::nullopt;
    }

    return wstr;
}
#else
static std::optional<std::string> ConvertString(const char* data,
                                                size_t length,
                                                const char* sourceEncoding,
                                                const char* targetEncoding) {
    if (length == 0) {
        return std::string();
    }

    iconv_t cd = iconv_open(targetEncoding, sourceEncoding);
    if (cd == ((iconv_t)-1)) {
        return std::nullopt;
    }
    struct ScopedIconvT {
        iconv_t cd;
        ~ScopedIconvT() {
            iconv_close(cd);
        }
    };
    ScopedIconvT scoped{cd};

    std::string result;
    std::array<char, 2048> buffer;
    size_t rest = length;
    char* next = const_cast<char*>(data);
    do {
        char* in = next;
        char* out = buffer.data();
        size_t outfree = buffer.size();
        size_t rv = iconv(cd, &in, &rest, &out, &outfree);
        if (out == buffer.data()) {
            break;
        }
        if (rv == ((size_t)-1) && errno != E2BIG) {
            return std::nullopt;
        }
        result.append(buffer.data(), out);
        if (in == next) {
            break;
        }
        next = in;
    } while (rest > 0);

    if (rest != 0) {
        return std::nullopt;
    }

    return result;
}
#endif

std::optional<std::string> Utf16ToUtf8(const char16_t* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    return WStringToCodepage(reinterpret_cast<const wchar_t*>(data), length, CP_UTF8);
#else
    return ConvertString(reinterpret_cast<const char*>(data), length * 2, "UTF-16LE", "UTF-8");
#endif
}

std::optional<std::u16string> Utf8ToUtf16(const char* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    return CodepageToString16<std::u16string>(data, length, CP_UTF8);
#else
    const auto str = ConvertString(data, length, "UTF-8", "UTF-16LE");
    if (!str || (str->size() % 2) != 0) {
        return std::nullopt;
    }
    std::u16string str16;
    str16.reserve(str->size() / 2);
    for (size_t i = 0; i < str->size(); i += 2) {
        const char16_t c =
            char16_t(uint16_t(uint8_t((*str)[i])) | (uint16_t(uint8_t((*str)[i + 1])) << 8));
        str16.push_back(c);
    }
    return str16;
#endif
}

std::optional<std::string> Utf16ToShiftJis(const char16_t* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    return WStringToCodepage(reinterpret_cast<const wchar_t*>(data), length, 932);
#else
    auto str8 = Utf16ToUtf8(data, length);
    if (!str8) {
        return std::nullopt;
    }
    return Utf8ToShiftJis(str8->data(), str8->size());
#endif
}

std::optional<std::u16string> ShiftJisToUtf16(const char* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    return CodepageToString16<std::u16string>(data, length, 932);
#else
    auto str8 = ShiftJisToUtf8(data, length);
    if (!str8) {
        return std::nullopt;
    }
    return Utf8ToUtf16(str8->data(), str8->size());
#endif
}

#ifdef BUILD_FOR_WINDOWS
std::optional<std::string> WStringToUtf8(const wchar_t* data, size_t length) {
    return WStringToCodepage(data, length, CP_UTF8);
}

std::optional<std::wstring> Utf8ToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, CP_UTF8);
}

std::optional<std::string> WStringToShiftJis(const wchar_t* data, size_t length) {
    return WStringToCodepage(data, length, 932);
}

std::optional<std::wstring> ShiftJisToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, 932);
}
#endif

std::optional<std::string> ShiftJisToUtf8(const char* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = ShiftJisToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToUtf8(wstr->data(), wstr->size());
#else
    return ConvertString(data, length, "CP932", "UTF-8");
#endif
}

std::optional<std::string> Utf8ToShiftJis(const char* data, size_t length) {
#ifdef BUILD_FOR_WINDOWS
    auto wstr = Utf8ToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToShiftJis(wstr->data(), wstr->size());
#else
    return ConvertString(data, length, "UTF-8", "CP932");
#endif
}

#ifdef BUILD_FOR_WINDOWS
std::optional<std::wstring> AnsiCodePageToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, GetACP());
}

std::optional<std::string> AnsiCodePageToUtf8(const char* data, size_t length) {
    auto wstr = AnsiCodePageToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToUtf8(wstr->data(), wstr->size());
}

std::optional<std::wstring> OemCodePageToWString(const char* data, size_t length) {
    return CodepageToString16<std::wstring>(data, length, GetOEMCP());
}

std::optional<std::string> OemCodePageToUtf8(const char* data, size_t length) {
    auto wstr = OemCodePageToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToUtf8(wstr->data(), wstr->size());
}
#endif

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

std::string ReplaceSubstring(std::string_view a, size_t offset, size_t length, std::string_view b) {
    std::string s;
    s += a.substr(0, offset);
    s += b;
    s += a.substr(offset + length);
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

std::string ToLower(std::string_view sv) {
    std::string result;
    result.reserve(sv.size());
    for (size_t i = 0; i < sv.size(); ++i) {
        result.push_back((sv[i] >= 'A' && sv[i] <= 'Z') ? (sv[i] + ('a' - 'A')) : sv[i]);
    }
    return result;
}

std::string ToUpper(std::string_view sv) {
    std::string result;
    result.reserve(sv.size());
    for (size_t i = 0; i < sv.size(); ++i) {
        result.push_back((sv[i] >= 'a' && sv[i] <= 'z') ? (sv[i] - ('a' - 'A')) : sv[i]);
    }
    return result;
}
} // namespace HyoutaUtils::TextUtils
