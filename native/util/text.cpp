#include "text.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "util/scope.h"
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
static size_t MeasureWStringToCodepage(const wchar_t* data, size_t length, UINT codepage) noexcept {
    if (length == 0) {
        return 0;
    }
    if (data == nullptr) {
        return INVALID_LENGTH;
    }
    if (length > INT32_MAX) {
        return INVALID_LENGTH;
    }

    BOOL usedDefaultChar = FALSE;
    const DWORD flags = (codepage == CP_UTF8) ? static_cast<DWORD>(WC_ERR_INVALID_CHARS)
                                              : static_cast<DWORD>(WC_NO_BEST_FIT_CHARS);
    const LPBOOL usedDefaultCharPtr = (codepage == CP_UTF8) ? nullptr : &usedDefaultChar;

    const auto requiredBytes = WideCharToMultiByte(
        codepage, flags, data, static_cast<int>(length), nullptr, 0, nullptr, usedDefaultCharPtr);
    if (requiredBytes <= 0 || usedDefaultChar != FALSE) {
        return INVALID_LENGTH;
    }
    return static_cast<size_t>(requiredBytes);
}

static size_t MeasureCodepageToWString(const char* data, size_t length, UINT codepage) noexcept {
    if (length == 0) {
        return 0;
    }
    if (data == nullptr) {
        return INVALID_LENGTH;
    }
    if (length > INT32_MAX) {
        return INVALID_LENGTH;
    }

    const auto requiredCodeUnits = MultiByteToWideChar(
        codepage, MB_ERR_INVALID_CHARS, data, static_cast<int>(length), nullptr, 0);
    if (requiredCodeUnits <= 0) {
        return INVALID_LENGTH;
    }
    return static_cast<size_t>(requiredCodeUnits);
}

#if 0 // I don't think I actually have enough information for this?
static size_t MeasureCodepageToCodepage(UINT outputCodepage,
                                        const char* data,
                                        size_t length,
                                        UINT inputCodepage) noexcept {
    if (length == 0) {
        return 0;
    }
    if (data == nullptr) {
        return INVALID_LENGTH;
    }
    if (length > INT32_MAX) {
        return INVALID_LENGTH;
    }

    // this needs to be one in two steps: first to utf16, then to the target codepage
    size_t totalRequiredBytes = 0;
    size_t rest = length;
    const char* current = data;
    std::array<wchar_t, 2048> buffer;
    while (true) {
        const auto writtenCodeUnits = MultiByteToWideChar(inputCodepage,
                                                          MB_ERR_INVALID_CHARS,
                                                          current,
                                                          static_cast<int>(rest),
                                                          buffer.data(),
                                                          buffer.size());
        if (writtenCodeUnits <= 0) {
            return INVALID_LENGTH;
        }

        BOOL usedDefaultChar = FALSE;
        const DWORD flags = (outputCodepage == CP_UTF8) ? static_cast<DWORD>(WC_ERR_INVALID_CHARS)
                                                        : static_cast<DWORD>(WC_NO_BEST_FIT_CHARS);
        const LPBOOL usedDefaultCharPtr = (outputCodepage == CP_UTF8) ? nullptr : &usedDefaultChar;
        const auto requiredBytes = WideCharToMultiByte(outputCodepage,
                                                       flags,
                                                       buffer.data(),
                                                       static_cast<int>(writtenCodeUnits),
                                                       nullptr,
                                                       0,
                                                       nullptr,
                                                       usedDefaultCharPtr);
        if (requiredBytes <= 0 || usedDefaultChar != FALSE) {
            return INVALID_LENGTH;
        }

        totalRequiredBytes += static_cast<size_t>(requiredBytes);
        // uuuh do i not have access to this info...?
        // if (static_cast<size_t>(processedBytes) >= rest) {
        //    break;
        // }
        // rest -= static_cast<size_t>(processedBytes);
        // current += processedBytes;
    }

    return totalRequiredBytes;
}
#endif

static size_t ConvertWStringToCodepage(char* outputData,
                                       size_t outputLength,
                                       UINT outputCodepage,
                                       const wchar_t* inputData,
                                       size_t inputLength) noexcept {
    if (inputLength == 0) {
        return 0;
    }
    if (inputData == nullptr) {
        return INVALID_LENGTH;
    }
    if (inputLength > INT32_MAX) {
        return INVALID_LENGTH;
    }
    if (outputLength > INT32_MAX) {
        return INVALID_LENGTH;
    }

    BOOL usedDefaultChar = FALSE;
    const DWORD flags = (outputCodepage == CP_UTF8) ? static_cast<DWORD>(WC_ERR_INVALID_CHARS)
                                                    : static_cast<DWORD>(WC_NO_BEST_FIT_CHARS);
    const LPBOOL usedDefaultCharPtr = (outputCodepage == CP_UTF8) ? nullptr : &usedDefaultChar;

    const auto resultBytes = WideCharToMultiByte(outputCodepage,
                                                 flags,
                                                 inputData,
                                                 static_cast<int>(inputLength),
                                                 outputData,
                                                 static_cast<int>(outputLength),
                                                 nullptr,
                                                 usedDefaultCharPtr);
    if (resultBytes <= 0 || usedDefaultChar != FALSE) {
        return INVALID_LENGTH;
    }
    return static_cast<size_t>(resultBytes);
}

static size_t ConvertCodepageToWString(wchar_t* outputData,
                                       size_t outputLength,
                                       const char* inputData,
                                       size_t inputLength,
                                       UINT inputCodepage) noexcept {
    if (inputLength == 0) {
        return 0;
    }
    if (inputData == nullptr) {
        return INVALID_LENGTH;
    }
    if (inputLength > INT32_MAX) {
        return INVALID_LENGTH;
    }
    if (outputLength > INT32_MAX) {
        return INVALID_LENGTH;
    }

    const auto resultCodeUnits = MultiByteToWideChar(inputCodepage,
                                                     MB_ERR_INVALID_CHARS,
                                                     inputData,
                                                     static_cast<int>(inputLength),
                                                     outputData,
                                                     static_cast<int>(outputLength));
    if (resultCodeUnits <= 0) {
        return INVALID_LENGTH;
    }
    return static_cast<size_t>(resultCodeUnits);
}

static size_t MeasureCodepageToCodepage(UINT outputCodepage,
                                        const char* inputData,
                                        size_t inputLength,
                                        UINT inputCodepage) noexcept {
    const size_t requiredCodeUnits =
        MeasureCodepageToWString(inputData, inputLength, inputCodepage);
    if (requiredCodeUnits == INVALID_LENGTH) {
        return INVALID_LENGTH;
    }

    std::array<wchar_t, 2048> stackBuffer;
    std::unique_ptr<wchar_t[]> heapBuffer;
    wchar_t* buffer;
    if (requiredCodeUnits <= stackBuffer.size()) {
        buffer = stackBuffer.data();
    } else {
        buffer = new (std::nothrow) wchar_t[requiredCodeUnits];
        if (!buffer) {
            return INVALID_LENGTH;
        }
        heapBuffer.reset(buffer);
    }

    const size_t usedCodeUnits =
        ConvertCodepageToWString(buffer, requiredCodeUnits, inputData, inputLength, inputCodepage);
    if (usedCodeUnits != requiredCodeUnits) {
        return INVALID_LENGTH;
    }

    return MeasureWStringToCodepage(buffer, requiredCodeUnits, outputCodepage);
}

static size_t ConvertCodepageToCodepage(char* outputData,
                                        size_t outputLength,
                                        UINT outputCodepage,
                                        const char* inputData,
                                        size_t inputLength,
                                        UINT inputCodepage) noexcept {
    const size_t requiredCodeUnits =
        MeasureCodepageToWString(inputData, inputLength, inputCodepage);
    if (requiredCodeUnits == INVALID_LENGTH) {
        return INVALID_LENGTH;
    }

    std::array<wchar_t, 2048> stackBuffer;
    std::unique_ptr<wchar_t[]> heapBuffer;
    wchar_t* buffer;
    if (requiredCodeUnits <= stackBuffer.size()) {
        buffer = stackBuffer.data();
    } else {
        buffer = new (std::nothrow) wchar_t[requiredCodeUnits];
        if (!buffer) {
            return INVALID_LENGTH;
        }
        heapBuffer.reset(buffer);
    }

    const size_t usedCodeUnits =
        ConvertCodepageToWString(buffer, requiredCodeUnits, inputData, inputLength, inputCodepage);
    if (usedCodeUnits != requiredCodeUnits) {
        return INVALID_LENGTH;
    }

    return ConvertWStringToCodepage(
        outputData, outputLength, outputCodepage, buffer, requiredCodeUnits);
}

static std::optional<std::string>
    WStringToCodepage_StdString(const wchar_t* data, size_t length, UINT codepage) noexcept {
    const size_t requiredBytes = MeasureWStringToCodepage(data, length, codepage);
    if (requiredBytes == INVALID_LENGTH) {
        return std::nullopt;
    }
    std::string result;
    try {
        result.resize(requiredBytes);
    } catch (...) {
        return std::nullopt;
    }
    const size_t convertedBytes =
        ConvertWStringToCodepage(result.data(), requiredBytes, codepage, data, length);
    if (convertedBytes != requiredBytes) {
        return std::nullopt;
    }
    return result;
}

template<typename T>
static std::optional<T>
    CodepageToWString_StdString16(const char* data, size_t length, UINT codepage) noexcept {
    static_assert(sizeof(typename T::value_type) == 2);
    const size_t requiredCodeUnits = MeasureCodepageToWString(data, length, codepage);
    if (requiredCodeUnits == INVALID_LENGTH) {
        return std::nullopt;
    }
    T result;
    try {
        result.resize(requiredCodeUnits);
    } catch (...) {
        return std::nullopt;
    }
    const size_t convertedCodeUnits = ConvertCodepageToWString(
        reinterpret_cast<wchar_t*>(result.data()), requiredCodeUnits, data, length, codepage);
    if (convertedCodeUnits != requiredCodeUnits) {
        return std::nullopt;
    }
    return result;
}
#else
static size_t MeasureStringConversionWithIconv(const char* inputData,
                                               size_t inputLength,
                                               iconv_t* cd) noexcept {
    size_t result = 0;
    std::array<char, 2048> buffer;
    size_t rest = inputLength;
    char* next = const_cast<char*>(inputData);
    do {
        char* in = next;
        char* out = buffer.data();
        size_t outfree = buffer.size();
        errno = 0;
        size_t rv = iconv(*cd, &in, &rest, &out, &outfree);
        const int err = errno;
        if (out == buffer.data()) {
            break;
        }
        if (rv == ((size_t)-1) && err != E2BIG) {
            return INVALID_LENGTH;
        }
        try {
            result += (out - buffer.data());
        } catch (...) {
            return INVALID_LENGTH;
        }
        if (in == next) {
            break;
        }
        next = in;
    } while (rest > 0);

    if (rest != 0) {
        return INVALID_LENGTH;
    }

    return result;
}

static size_t MeasureStringConversion(const char* outputEncoding,
                                      const char* inputData,
                                      size_t inputLength,
                                      const char* inputEncoding) noexcept {
    if (inputLength == 0) {
        return 0;
    }
    if (inputData == nullptr) {
        return INVALID_LENGTH;
    }

    iconv_t cd = iconv_open(outputEncoding, inputEncoding);
    if (cd == ((iconv_t)-1)) {
        return INVALID_LENGTH;
    }
    auto scope = HyoutaUtils::MakeScopeGuard([&]() { iconv_close(cd); });

    return MeasureStringConversionWithIconv(inputData, inputLength, &cd);
}

static size_t ConvertStringWithIconv(char* outputData,
                                     size_t outputLength,
                                     const char* inputData,
                                     size_t inputLength,
                                     iconv_t* cd) noexcept {
    char* in = const_cast<char*>(inputData);
    size_t inRest = inputLength;
    char* out = outputData;
    size_t outRest = outputLength;
    errno = 0;
    size_t rv = iconv(*cd, &in, &inRest, &out, &outRest);
    const int err = errno;
    if (rv == ((size_t)-1)) {
        return INVALID_LENGTH;
    }
    if (err != 0) {
        return INVALID_LENGTH;
    }
    return (out - outputData);
}

static size_t ConvertString(char* outputData,
                            size_t outputLength,
                            const char* outputEncoding,
                            const char* inputData,
                            size_t inputLength,
                            const char* inputEncoding) noexcept {
    if (inputLength == 0) {
        return 0;
    }
    if (inputData == nullptr) {
        return INVALID_LENGTH;
    }

    iconv_t cd = iconv_open(outputEncoding, inputEncoding);
    if (cd == ((iconv_t)-1)) {
        return INVALID_LENGTH;
    }
    auto scope = HyoutaUtils::MakeScopeGuard([&]() { iconv_close(cd); });

    return ConvertStringWithIconv(outputData, outputLength, inputData, inputLength, &cd);
}

static std::optional<std::string> ConvertString_StdString(const char* outputEncoding,
                                                          const char* inputData,
                                                          size_t inputLength,
                                                          const char* inputEncoding) noexcept {
    if (inputLength == 0) {
        return std::string();
    }
    if (inputData == nullptr) {
        return std::nullopt;
    }

    iconv_t cd = iconv_open(outputEncoding, inputEncoding);
    if (cd == ((iconv_t)-1)) {
        return std::nullopt;
    }
    auto scope = HyoutaUtils::MakeScopeGuard([&]() { iconv_close(cd); });

    const size_t requiredBytes = MeasureStringConversionWithIconv(inputData, inputLength, &cd);
    if (requiredBytes == INVALID_LENGTH) {
        return std::nullopt;
    }
    std::string result;
    try {
        result.resize(requiredBytes);
    } catch (...) {
        return std::nullopt;
    }
    iconv(cd, nullptr, nullptr, nullptr, nullptr); // reset iconv state
    const size_t convertedBytes =
        ConvertStringWithIconv(result.data(), requiredBytes, inputData, inputLength, &cd);
    if (convertedBytes != requiredBytes) {
        return std::nullopt;
    }
    return result;
}

template<typename T>
static std::optional<T> ConvertString_StdString16(const char* outputEncoding,
                                                  const char* inputData,
                                                  size_t inputLength,
                                                  const char* inputEncoding) noexcept {
    static_assert(sizeof(typename T::value_type) == 2);
    if (inputLength == 0) {
        return T();
    }
    if (inputData == nullptr) {
        return std::nullopt;
    }

    iconv_t cd = iconv_open(outputEncoding, inputEncoding);
    if (cd == ((iconv_t)-1)) {
        return std::nullopt;
    }
    auto scope = HyoutaUtils::MakeScopeGuard([&]() { iconv_close(cd); });

    const size_t requiredBytes = MeasureStringConversionWithIconv(inputData, inputLength, &cd);
    if (requiredBytes == INVALID_LENGTH) {
        return std::nullopt;
    }
    if ((requiredBytes % 2) != 0) {
        return std::nullopt;
    }
    T result;
    try {
        result.resize(requiredBytes / 2);
    } catch (...) {
        return std::nullopt;
    }
    iconv(cd, nullptr, nullptr, nullptr, nullptr); // reset iconv state
    const size_t convertedBytes = ConvertStringWithIconv(
        reinterpret_cast<char*>(result.data()), requiredBytes, inputData, inputLength, &cd);
    if (convertedBytes != requiredBytes) {
        return std::nullopt;
    }
    return result;
}
#endif

size_t MeasureUtf16ToUtf8(const char16_t* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureWStringToCodepage(
        reinterpret_cast<const wchar_t*>(inputData), inputLength, CP_UTF8);
#else
    return MeasureStringConversion(
        "UTF-8", reinterpret_cast<const char*>(inputData), inputLength * 2, "UTF-16LE");
#endif
}

size_t MeasureUtf8ToUtf16(const char* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureCodepageToWString(inputData, inputLength, CP_UTF8);
#else
    const size_t result = MeasureStringConversion("UTF-16LE", inputData, inputLength, "UTF-8");
    if ((result % 2) != 0) {
        return INVALID_LENGTH;
    }
    return result / 2;
#endif
}

size_t MeasureUtf16ToShiftJis(const char16_t* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureWStringToCodepage(reinterpret_cast<const wchar_t*>(inputData), inputLength, 932);
#else
    return MeasureStringConversion(
        "CP932", reinterpret_cast<const char*>(inputData), inputLength * 2, "UTF-16LE");
#endif
}

size_t MeasureShiftJisToUtf16(const char* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureCodepageToWString(inputData, inputLength, 932);
#else
    const size_t result = MeasureStringConversion("UTF-16LE", inputData, inputLength, "CP932");
    if ((result % 2) != 0) {
        return INVALID_LENGTH;
    }
    return result / 2;
#endif
}

size_t MeasureShiftJisToUtf8(const char* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureCodepageToCodepage(CP_UTF8, inputData, inputLength, 932);
#else
    return MeasureStringConversion("UTF-8", inputData, inputLength, "CP932");
#endif
}

size_t MeasureUtf8ToShiftJis(const char* inputData, size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return MeasureCodepageToCodepage(932, inputData, inputLength, CP_UTF8);
#else
    return MeasureStringConversion("CP932", inputData, inputLength, "UTF-8");
#endif
}

#ifdef BUILD_FOR_WINDOWS
size_t MeasureWStringToUtf8(const wchar_t* inputData, size_t inputLength) noexcept {
    return MeasureWStringToCodepage(inputData, inputLength, CP_UTF8);
}

size_t MeasureUtf8ToWString(const char* inputData, size_t inputLength) noexcept {
    return MeasureCodepageToWString(inputData, inputLength, CP_UTF8);
}

size_t MeasureWStringToShiftJis(const wchar_t* inputData, size_t inputLength) noexcept {
    return MeasureWStringToCodepage(inputData, inputLength, 932);
}

size_t MeasureShiftJisToWString(const char* inputData, size_t inputLength) noexcept {
    return MeasureCodepageToWString(inputData, inputLength, 932);
}
#endif

size_t ConvertUtf16ToUtf8(char* outputData,
                          size_t outputLength,
                          const char16_t* inputData,
                          size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertWStringToCodepage(outputData,
                                    outputLength,
                                    CP_UTF8,
                                    reinterpret_cast<const wchar_t*>(inputData),
                                    inputLength);
#else
    return ConvertString(outputData,
                         outputLength,
                         "UTF-8",
                         reinterpret_cast<const char*>(inputData),
                         inputLength * 2,
                         "UTF-16LE");
#endif
}

size_t ConvertUtf8ToUtf16(char16_t* outputData,
                          size_t outputLength,
                          const char* inputData,
                          size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertCodepageToWString(
        reinterpret_cast<wchar_t*>(outputData), outputLength, inputData, inputLength, CP_UTF8);
#else
    size_t result = ConvertString(reinterpret_cast<char*>(outputData),
                                  outputLength * 2,
                                  "UTF-16LE",
                                  inputData,
                                  inputLength,
                                  "UTF-8");
    if ((result % 2) != 0) {
        return INVALID_LENGTH;
    }
    return result / 2;
#endif
}

size_t ConvertUtf16ToShiftJis(char* outputData,
                              size_t outputLength,
                              const char16_t* inputData,
                              size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertWStringToCodepage(
        outputData, outputLength, 932, reinterpret_cast<const wchar_t*>(inputData), inputLength);
#else
    return ConvertString(outputData,
                         outputLength,
                         "CP932",
                         reinterpret_cast<const char*>(inputData),
                         inputLength * 2,
                         "UTF-16LE");
#endif
}

size_t ConvertShiftJisToUtf16(char16_t* outputData,
                              size_t outputLength,
                              const char* inputData,
                              size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertCodepageToWString(
        reinterpret_cast<wchar_t*>(outputData), outputLength, inputData, inputLength, 932);
#else
    size_t result = ConvertString(reinterpret_cast<char*>(outputData),
                                  outputLength * 2,
                                  "UTF-16LE",
                                  inputData,
                                  inputLength,
                                  "CP932");
    if ((result % 2) != 0) {
        return INVALID_LENGTH;
    }
    return result / 2;
#endif
}

size_t ConvertShiftJisToUtf8(char* outputData,
                             size_t outputLength,
                             const char* inputData,
                             size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertCodepageToCodepage(
        outputData, outputLength, CP_UTF8, inputData, inputLength, 932);
#else
    return ConvertString(outputData, outputLength, "UTF-8", inputData, inputLength, "CP932");
#endif
}

size_t ConvertUtf8ToShiftJis(char* outputData,
                             size_t outputLength,
                             const char* inputData,
                             size_t inputLength) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return ConvertCodepageToCodepage(
        outputData, outputLength, 932, inputData, inputLength, CP_UTF8);
#else
    return ConvertString(outputData, outputLength, "CP932", inputData, inputLength, "UTF-8");
#endif
}

#ifdef BUILD_FOR_WINDOWS
size_t ConvertWStringToUtf8(char* outputData,
                            size_t outputLength,
                            const wchar_t* inputData,
                            size_t inputLength) noexcept {
    return ConvertWStringToCodepage(outputData, outputLength, CP_UTF8, inputData, inputLength);
}

size_t ConvertUtf8ToWString(wchar_t* outputData,
                            size_t outputLength,
                            const char* inputData,
                            size_t inputLength) noexcept {
    return ConvertCodepageToWString(outputData, outputLength, inputData, inputLength, CP_UTF8);
}

size_t ConvertWStringToShiftJis(char* outputData,
                                size_t outputLength,
                                const wchar_t* inputData,
                                size_t inputLength) noexcept {
    return ConvertWStringToCodepage(outputData, outputLength, 932, inputData, inputLength);
}

size_t ConvertShiftJisToWString(wchar_t* outputData,
                                size_t outputLength,
                                const char* inputData,
                                size_t inputLength) noexcept {
    return ConvertCodepageToWString(outputData, outputLength, inputData, inputLength, 932);
}
#endif

std::optional<std::string> Utf16ToUtf8(const char16_t* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return WStringToCodepage_StdString(reinterpret_cast<const wchar_t*>(data), length, CP_UTF8);
#else
    return ConvertString_StdString(
        "UTF-8", reinterpret_cast<const char*>(data), length * 2, "UTF-16LE");
#endif
}

std::optional<std::u16string> Utf8ToUtf16(const char* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return CodepageToWString_StdString16<std::u16string>(data, length, CP_UTF8);
#else
    return ConvertString_StdString16<std::u16string>("UTF-16LE", data, length, "UTF-8");
#endif
}

std::optional<std::string> Utf16ToShiftJis(const char16_t* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return WStringToCodepage_StdString(reinterpret_cast<const wchar_t*>(data), length, 932);
#else
    return ConvertString_StdString(
        "CP932", reinterpret_cast<const char*>(data), length * 2, "UTF-16LE");
#endif
}

std::optional<std::u16string> ShiftJisToUtf16(const char* data, size_t length) noexcept {
#ifdef BUILD_FOR_WINDOWS
    return CodepageToWString_StdString16<std::u16string>(data, length, 932);
#else
    return ConvertString_StdString16<std::u16string>("UTF-16LE", data, length, "CP932");
#endif
}

#ifdef BUILD_FOR_WINDOWS
std::optional<std::string> WStringToUtf8(const wchar_t* data, size_t length) noexcept {
    return WStringToCodepage_StdString(data, length, CP_UTF8);
}

std::optional<std::wstring> Utf8ToWString(const char* data, size_t length) noexcept {
    return CodepageToWString_StdString16<std::wstring>(data, length, CP_UTF8);
}

std::optional<std::string> WStringToShiftJis(const wchar_t* data, size_t length) noexcept {
    return WStringToCodepage_StdString(data, length, 932);
}

std::optional<std::wstring> ShiftJisToWString(const char* data, size_t length) noexcept {
    return CodepageToWString_StdString16<std::wstring>(data, length, 932);
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
    return ConvertString_StdString("UTF-8", data, length, "CP932");
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
    return ConvertString_StdString("CP932", data, length, "UTF-8");
#endif
}

#ifdef BUILD_FOR_WINDOWS
std::optional<std::wstring> AnsiCodePageToWString(const char* data, size_t length) noexcept {
    return CodepageToWString_StdString16<std::wstring>(data, length, GetACP());
}

std::optional<std::string> AnsiCodePageToUtf8(const char* data, size_t length) noexcept {
    auto wstr = AnsiCodePageToWString(data, length);
    if (!wstr) {
        return std::nullopt;
    }
    return WStringToUtf8(wstr->data(), wstr->size());
}

std::optional<std::wstring> OemCodePageToWString(const char* data, size_t length) noexcept {
    return CodepageToWString_StdString16<std::wstring>(data, length, GetOEMCP());
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

bool CaseInsensitiveContains(std::string_view haystack, std::string_view needle) noexcept {
    if (needle.size() > haystack.size()) {
        return false;
    }
    const size_t len = needle.size();
    const size_t loops = (haystack.size() - needle.size()) + 1;
    const char* lhs = haystack.data();
    const char* rhs = needle.data();
    for (size_t loop = 0; loop < loops; ++loop) {
        bool matches = true;
        for (size_t i = 0; i < len; ++i) {
            const char cl = (lhs[i] >= 'A' && lhs[i] <= 'Z') ? (lhs[i] + ('a' - 'A')) : lhs[i];
            const char cr = (rhs[i] >= 'A' && rhs[i] <= 'Z') ? (rhs[i] + ('a' - 'A')) : rhs[i];
            if (cl != cr) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return true;
        }
        ++lhs;
    }
    return false;
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
