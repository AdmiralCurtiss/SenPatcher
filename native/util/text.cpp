#include "text.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "util/vector.h"

#ifdef BUILD_FOR_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <iconv.h>
#endif

namespace HyoutaUtils::TextUtils {
#ifdef BUILD_FOR_WINDOWS
static std::optional<std::string>
    WStringToCodepage(const wchar_t* data, size_t length, UINT codepage) noexcept {
    if (length == 0) {
        return std::string();
    }

    if (length > INT32_MAX) {
        return std::nullopt;
    }

    BOOL usedDefaultChar = FALSE;
    const DWORD flags = (codepage == CP_UTF8) ? static_cast<DWORD>(WC_ERR_INVALID_CHARS)
                                              : static_cast<DWORD>(WC_NO_BEST_FIT_CHARS);
    const LPBOOL usedDefaultCharPtr = (codepage == CP_UTF8) ? nullptr : &usedDefaultChar;

    const auto requiredBytes = WideCharToMultiByte(
        codepage, flags, data, static_cast<int>(length), nullptr, 0, nullptr, usedDefaultCharPtr);
    if (requiredBytes <= 0 || usedDefaultChar != FALSE) {
        return std::nullopt;
    }

    std::string result;
    try {
        result.resize(static_cast<size_t>(requiredBytes));
    } catch (...) {
        return std::nullopt;
    }
    const auto convertedBytes = WideCharToMultiByte(codepage,
                                                    flags,
                                                    data,
                                                    static_cast<int>(length),
                                                    result.data(),
                                                    requiredBytes,
                                                    nullptr,
                                                    usedDefaultCharPtr);
    if (convertedBytes != requiredBytes || usedDefaultChar != FALSE) {
        return std::nullopt;
    }

    return result;
}

template<typename T>
static std::optional<T>
    CodepageToString16(const char* data, size_t length, UINT codepage) noexcept {
    static_assert(sizeof(typename T::value_type) == 2);

    if (length == 0) {
        return T();
    }

    if (length > INT32_MAX) {
        return std::nullopt;
    }

    const auto requiredBytes = MultiByteToWideChar(
        codepage, MB_ERR_INVALID_CHARS, data, static_cast<int>(length), nullptr, 0);
    if (requiredBytes <= 0) {
        return std::nullopt;
    }

    T wstr;
    try {
        wstr.resize(static_cast<size_t>(requiredBytes));
    } catch (...) {
        return std::nullopt;
    }
    const auto convertedBytes = MultiByteToWideChar(codepage,
                                                    MB_ERR_INVALID_CHARS,
                                                    data,
                                                    static_cast<int>(length),
                                                    (wchar_t*)wstr.data(),
                                                    requiredBytes);
    if (convertedBytes != requiredBytes) {
        return std::nullopt;
    }

    return wstr;
}
#else
static std::optional<std::string> ConvertString(const char* data,
                                                size_t length,
                                                const char* sourceEncoding,
                                                const char* targetEncoding) noexcept {
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
        errno = 0;
        size_t rv = iconv(cd, &in, &rest, &out, &outfree);
        if (out == buffer.data()) {
            break;
        }
        if (rv == ((size_t)-1) && errno != E2BIG) {
            return std::nullopt;
        }
        try {
            result.append(buffer.data(), out);
        } catch (...) {
            return std::nullopt;
        }
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

std::optional<std::string> Utf16ToUtf8(const char16_t* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return WStringToCodepage(reinterpret_cast<const wchar_t*>(data), length, CP_UTF8);
#else
    return ConvertString(reinterpret_cast<const char*>(data), length * 2, "UTF-16LE", "UTF-8");
#endif
}

std::optional<std::u16string> Utf8ToUtf16(const char* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return CodepageToString16<std::u16string>(data, length, CP_UTF8);
#else
    const auto str = ConvertString(data, length, "UTF-8", "UTF-16LE");
    if (!str || (str->size() % 2) != 0) {
        return std::nullopt;
    }
    std::u16string str16;
    try {
        str16.reserve(str->size() / 2);
        for (size_t i = 0; i < str->size(); i += 2) {
            const char16_t c =
                char16_t(uint16_t(uint8_t((*str)[i])) | (uint16_t(uint8_t((*str)[i + 1])) << 8));
            str16.push_back(c);
        }
    } catch (...) {
        return std::nullopt;
    }
    return str16;
#endif
}

std::optional<std::string> Utf16ToShiftJis(const char16_t* data, size_t length) noexcept {
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

std::optional<std::u16string> ShiftJisToUtf16(const char* data, size_t length) noexcept {
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
std::optional<std::string> WStringToUtf8(const wchar_t* data, size_t length) noexcept {
    return WStringToCodepage(data, length, CP_UTF8);
}

std::optional<std::wstring> Utf8ToWString(const char* data, size_t length) noexcept {
    return CodepageToString16<std::wstring>(data, length, CP_UTF8);
}

std::optional<std::string> WStringToShiftJis(const wchar_t* data, size_t length) noexcept {
    return WStringToCodepage(data, length, 932);
}

std::optional<std::wstring> ShiftJisToWString(const char* data, size_t length) noexcept {
    return CodepageToString16<std::wstring>(data, length, 932);
}
#endif

std::optional<std::string> ShiftJisToUtf8(const char* data, size_t length) noexcept {
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

std::optional<std::string> Utf8ToShiftJis(const char* data, size_t length) noexcept {
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
std::optional<std::wstring> AnsiCodePageToWString(const char* data, size_t length) noexcept {
    return CodepageToString16<std::wstring>(data, length, GetACP());
}

std::optional<std::string> AnsiCodePageToUtf8(const char* data, size_t length) noexcept {
    auto wstr = AnsiCodePageToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToUtf8(wstr->data(), wstr->size());
}

std::optional<std::wstring> OemCodePageToWString(const char* data, size_t length) noexcept {
    return CodepageToString16<std::wstring>(data, length, GetOEMCP());
}

std::optional<std::string> OemCodePageToUtf8(const char* data, size_t length) noexcept {
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

std::string MoveSubstring(std::string_view input,
                          size_t sourceLocation,
                          size_t targetLocation,
                          size_t length) {
    std::string result(input);
    HyoutaUtils::Vector::ShiftData(std::span<char>(result.data(), result.data() + result.size()),
                                   sourceLocation,
                                   targetLocation,
                                   length);
    return result;
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

static bool IsWhitespace(char c) noexcept {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

std::string_view Trim(std::string_view sv) noexcept {
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

std::string Join(const std::vector<std::string>& svs, std::string_view joinPattern) {
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

bool CaseInsensitiveEquals(std::string_view lhs, std::string_view rhs) noexcept {
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

int CaseInsensitiveCompare(std::string_view lhs, std::string_view rhs) noexcept {
    const size_t shorter = std::min(lhs.size(), rhs.size());
    for (size_t i = 0; i < shorter; ++i) {
        const uint8_t cl = static_cast<uint8_t>(
            (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i]);
        const uint8_t cr = static_cast<uint8_t>(
            (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i]);
        if (cl < cr) {
            return -1;
        }
        if (cl > cr) {
            return 1;
        }
    }
    if (lhs.size() < rhs.size()) {
        return -1;
    }
    if (lhs.size() > rhs.size()) {
        return 1;
    }
    return 0;
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

template<typename CharMatchF>
static bool MatchesSameLengthNoAsterik(const char* string,
                                       const char* glob,
                                       size_t length,
                                       const CharMatchF& charMatchFunc) {
    for (size_t i = 0; i < length; ++i) {
        const char c = *(glob + i);
        if (!(c == '?' || charMatchFunc(c, *(string + i)))) {
            return false;
        }
    }
    return true;
}

template<typename CharMatchF>
static size_t FindLastMatchNoAsterik(std::string_view string,
                                     std::string_view glob,
                                     const CharMatchF& charMatchFunc) {
    if (glob.size() > string.size()) {
        return std::string_view::npos;
    }

    size_t pos = string.size() - glob.size();
    while (true) {
        if (MatchesSameLengthNoAsterik(
                string.data() + pos, glob.data(), glob.size(), charMatchFunc)) {
            return pos;
        }
        if (pos == 0) {
            return std::string_view::npos;
        }
        --pos;
    }
}

template<typename CharMatchF>
static bool GlobMatchesTemplate(std::string_view string,
                                std::string_view glob,
                                const CharMatchF& charMatchFunc) noexcept {
    std::string_view restString = string;
    std::string_view restGlob = glob;

    while (true) {
        // strip front to first asterik
        while (!restGlob.empty()) {
            const char c = restGlob.front();
            if (c == '*') {
                break;
            }
            if (restString.empty()) {
                return false;
            }
            if (c == '?' || charMatchFunc(c, restString.front())) {
                restGlob = restGlob.substr(1);
                restString = restString.substr(1);
                continue;
            }
            return false;
        }

        // strip back to last asterik
        while (!restGlob.empty()) {
            const char c = restGlob.back();
            if (c == '*') {
                break;
            }
            if (restString.empty()) {
                return false;
            }
            if (c == '?' || charMatchFunc(c, restString.back())) {
                restGlob = restGlob.substr(0, restGlob.size() - 1);
                restString = restString.substr(0, restString.size() - 1);
                continue;
            }
            return false;
        }

        // collapse sequential asteriks in front
        while (restGlob.size() > 1) {
            if (restGlob.front() == '*' && restGlob[1] == '*') {
                restGlob = restGlob.substr(1);
            } else {
                break;
            }
        }

        // collapse sequential asteriks in back
        while (restGlob.size() > 1) {
            if (restGlob.back() == '*' && restGlob[restGlob.size() - 2] == '*') {
                restGlob = restGlob.substr(0, restGlob.size() - 1);
            } else {
                break;
            }
        }

        // if the glob is now empty it only matches the empty string
        if (restGlob.empty()) {
            return restString.empty();
        }

        if (restGlob.size() == 1) {
            // this means that restGlob is a single asterik
            return true;
        }

        // this means we have something like "*stuff*" or "*?anything?*" or "*abc*def*".

        // extract the next non-* string from restGlob
        // note that since we know there is a '*' at the end of the string we don't need to check if
        // we actually find one, it's guaranteed to exist
        std::string_view nextGlobSeq = restGlob.substr(1, restGlob.find_first_of('*', 1) - 1);
        const size_t lastMatchAt = FindLastMatchNoAsterik(restString, nextGlobSeq, charMatchFunc);
        if (lastMatchAt == std::string_view::npos) {
            return false;
        }

        // asterik matched a sequence of characters, strip and restart
        restString = restString.substr(lastMatchAt + nextGlobSeq.size());
        restGlob = restGlob.substr(nextGlobSeq.size() + 1);

        continue;
    }
}

bool GlobMatches(std::string_view string, std::string_view glob) noexcept {
    return GlobMatchesTemplate(
        string, glob, [](const char lhs, const char rhs) { return lhs == rhs; });
}

bool CaseInsensitiveGlobMatches(std::string_view string, std::string_view glob) noexcept {
    return GlobMatchesTemplate(string, glob, [](const char lhs, const char rhs) {
        const char cl = (lhs >= 'A' && lhs <= 'Z') ? (lhs + ('a' - 'A')) : lhs;
        const char cr = (rhs >= 'A' && rhs <= 'Z') ? (rhs + ('a' - 'A')) : rhs;
        return cl == cr;
    });
}
} // namespace HyoutaUtils::TextUtils
