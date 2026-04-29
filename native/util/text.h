#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
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

static constexpr size_t INVALID_LENGTH = std::numeric_limits<size_t>::max();

// Returns the number of bytes the UTF8-encoded version of the given UTF16LE-encoded string takes.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureUtf16ToUtf8(const char16_t* inputData, size_t inputLength) noexcept;

// Returns the number of UTF16 code units (one code unit is 16 bits) the UTF16-encoded version of
// the given UTF8-encoded string takes. inputLength is in number of bytes.
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureUtf8ToUtf16(const char* inputData, size_t inputLength) noexcept;

// Returns the number of bytes the Windows-932-encoded version of the given UTF16LE-encoded string
// takes. inputLength is in number of UTF16 code units (one code unit is 16 bits).
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureUtf16ToShiftJis(const char16_t* inputData, size_t inputLength) noexcept;

// Returns the number of UTF16 code units (one code unit is 16 bits) the UTF16-encoded version of
// the given Windows-932-encoded string takes. inputLength is in number of bytes.
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureShiftJisToUtf16(const char* inputData, size_t inputLength) noexcept;

// Returns the number of bytes the UTF8-encoded version of the given Windows-932-encoded string
// takes. inputLength is in number of bytes. Returns INVALID_LENGTH on any kind of failure.
size_t MeasureShiftJisToUtf8(const char* inputData, size_t inputLength) noexcept;

// Returns the number of bytes the Windows-932-encoded version of the given UTF8-encoded string
// takes. inputLength is in number of bytes. Returns INVALID_LENGTH on any kind of failure.
size_t MeasureUtf8ToShiftJis(const char* inputData, size_t inputLength) noexcept;

#ifdef BUILD_FOR_WINDOWS
// Returns the number of bytes the UTF8-encoded version of the given UTF16LE-encoded string takes.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureWStringToUtf8(const wchar_t* inputData, size_t inputLength) noexcept;

// Returns the number of UTF16 code units (one code unit is 16 bits) the UTF16-encoded version of
// the given UTF8-encoded string takes. inputLength is in number of bytes.
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureUtf8ToWString(const char* inputData, size_t inputLength) noexcept;

// Returns the number of bytes the Windows-932-encoded version of the given UTF16LE-encoded string
// takes. inputLength is in number of UTF16 code units (one code unit is 16 bits).
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureWStringToShiftJis(const wchar_t* inputData, size_t inputLength) noexcept;

// Returns the number of UTF16 code units (one code unit is 16 bits) the UTF16-encoded version of
// the given Windows-932-encoded string takes. inputLength is in number of bytes.
// Returns INVALID_LENGTH on any kind of failure.
size_t MeasureShiftJisToWString(const char* inputData, size_t inputLength) noexcept;
#endif

// Writes the UTF8-encoded version of the given UTF16LE-encoded string into outputData.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertUtf16ToUtf8(char* outputData,
                          size_t outputLength,
                          const char16_t* inputData,
                          size_t inputLength) noexcept;

// Writes the UTF16LE-encoded version of the given UTF8-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of UTF16 code units (one code unit is 16 bits) at outputData.
// Returns the number of code units written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertUtf8ToUtf16(char16_t* outputData,
                          size_t outputLength,
                          const char* inputData,
                          size_t inputLength) noexcept;

// Writes the Windows-932-encoded version of the given UTF16LE-encoded string into outputData.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertUtf16ToShiftJis(char* outputData,
                              size_t outputLength,
                              const char16_t* inputData,
                              size_t inputLength) noexcept;

// Writes the UTF16LE-encoded version of the given Windows-932-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of UTF16 code units (one code unit is 16 bits) at outputData.
// Returns the number of code units written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertShiftJisToUtf16(char16_t* outputData,
                              size_t outputLength,
                              const char* inputData,
                              size_t inputLength) noexcept;

// Writes the UTF8-encoded version of the given Windows-932-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertShiftJisToUtf8(char* outputData,
                             size_t outputLength,
                             const char* inputData,
                             size_t inputLength) noexcept;

// Writes the Windows-932-encoded version of the given UTF8-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertUtf8ToShiftJis(char* outputData,
                             size_t outputLength,
                             const char* inputData,
                             size_t inputLength) noexcept;

#ifdef BUILD_FOR_WINDOWS
// Writes the UTF8-encoded version of the given UTF16LE-encoded string into outputData.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertWStringToUtf8(char* outputData,
                            size_t outputLength,
                            const wchar_t* inputData,
                            size_t inputLength) noexcept;

// Writes the UTF16LE-encoded version of the given UTF8-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of UTF16 code units (one code unit is 16 bits) at outputData.
// Returns the number of code units written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertUtf8ToWString(wchar_t* outputData,
                            size_t outputLength,
                            const char* inputData,
                            size_t inputLength) noexcept;

// Writes the Windows-932-encoded version of the given UTF16LE-encoded string into outputData.
// inputLength is in number of UTF16 code units (one code unit is 16 bits).
// outputLength is the number of available bytes at outputData.
// Returns the number of bytes written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertWStringToShiftJis(char* outputData,
                                size_t outputLength,
                                const wchar_t* inputData,
                                size_t inputLength) noexcept;

// Writes the UTF16LE-encoded version of the given Windows-932-encoded string into outputData.
// inputLength is in number of bytes.
// outputLength is the number of UTF16 code units (one code unit is 16 bits) at outputData.
// Returns the number of code units written into outputData.
// Returns INVALID_LENGTH on any kind of failure.
size_t ConvertShiftJisToWString(wchar_t* outputData,
                                size_t outputLength,
                                const char* inputData,
                                size_t inputLength) noexcept;
#endif

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
bool CaseInsensitiveContains(std::string_view haystack, std::string_view needle) noexcept;
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
