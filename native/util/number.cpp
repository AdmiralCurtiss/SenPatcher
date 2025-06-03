#include "number.h"

#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>

namespace HyoutaUtils::NumberUtils {
template<typename T>
static std::optional<T> ParseUnsignedInteger(std::string_view str) {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_unsigned_v<T>);
    const char* first = str.data();
    const char* const last = first + str.size();
    int base = 10;
    if (str.size() >= 2 && *first == '0') {
        const char prefixChar = *(first + 1);
        if (prefixChar == 'x' || prefixChar == 'X') {
            base = 16;
            first += 2;
        } else if (prefixChar == 'b' || prefixChar == 'B') {
            base = 2;
            first += 2;
        }
    }
    T t = 0;
    const auto [ptr, ec] = std::from_chars(first, last, t, base);
    if (ec != std::errc() || ptr != last) {
        return std::nullopt;
    }
    return t;
}

template<typename T>
static std::optional<T> ParseSignedInteger(std::string_view str) {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);
    using U = std::make_unsigned<T>::type;

    if (str.empty()) {
        return std::nullopt;
    }
    const char* first = str.data();
    const char* const last = first + str.size();
    const bool isNegative = (*first == '-');
    if (isNegative) {
        ++first;
    }
    const std::optional<U> parsed = ParseUnsignedInteger<U>(std::string_view(first, last));
    if (!parsed.has_value()) {
        return std::nullopt;
    }

    const U u = (isNegative ? static_cast<U>(static_cast<U>(0) - *parsed) : *parsed);
    const T t = std::bit_cast<T>(u);
    if (isNegative != (t < 0)) {
        return std::nullopt;
    }
    return t;
}

std::optional<int8_t> ParseInt8(std::string_view str) {
    return ParseSignedInteger<int8_t>(str);
}
std::optional<uint8_t> ParseUInt8(std::string_view str) {
    return ParseUnsignedInteger<uint8_t>(str);
}
std::optional<int16_t> ParseInt16(std::string_view str) {
    return ParseSignedInteger<int16_t>(str);
}
std::optional<uint16_t> ParseUInt16(std::string_view str) {
    return ParseUnsignedInteger<uint16_t>(str);
}
std::optional<int32_t> ParseInt32(std::string_view str) {
    return ParseSignedInteger<int32_t>(str);
}
std::optional<uint32_t> ParseUInt32(std::string_view str) {
    return ParseUnsignedInteger<uint32_t>(str);
}
std::optional<int64_t> ParseInt64(std::string_view str) {
    return ParseSignedInteger<int64_t>(str);
}
std::optional<uint64_t> ParseUInt64(std::string_view str) {
    return ParseUnsignedInteger<uint64_t>(str);
}
std::optional<std::size_t> ParseSizeT(std::string_view str) {
    return ParseUnsignedInteger<std::size_t>(str);
}

template<typename T>
static std::optional<T> ParseFloatingPoint(std::string_view str) {
    static_assert(std::is_floating_point_v<T>);
    const char* first = str.data();
    const char* const last = first + str.size();
    T t(0);
    const auto [ptr, ec] = std::from_chars(first, last, t);
    if (ec != std::errc() || ptr != last) {
        return std::nullopt;
    }
    return t;
}

std::optional<float> ParseFloat(std::string_view str) {
    return ParseFloatingPoint<float>(str);
}
std::optional<double> ParseDouble(std::string_view str) {
    return ParseFloatingPoint<double>(str);
}
} // namespace HyoutaUtils::NumberUtils
