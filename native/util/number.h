#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace HyoutaUtils::NumberUtils {
// All of these integer parsing functions expect just the number to parse in the string, no leading
// or trailing anything. "0x" and "0b" prefixes for base 16 and base 2 are allowed, otherwise base
// 10 is assumed.

std::optional<int8_t> ParseInt8(std::string_view str);
std::optional<uint8_t> ParseUInt8(std::string_view str);
std::optional<int16_t> ParseInt16(std::string_view str);
std::optional<uint16_t> ParseUInt16(std::string_view str);
std::optional<int32_t> ParseInt32(std::string_view str);
std::optional<uint32_t> ParseUInt32(std::string_view str);
std::optional<int64_t> ParseInt64(std::string_view str);
std::optional<uint64_t> ParseUInt64(std::string_view str);
std::optional<std::size_t> ParseSizeT(std::string_view str);

std::optional<float> ParseFloat(std::string_view str);
std::optional<double> ParseDouble(std::string_view str);
} // namespace HyoutaUtils::NumberUtils
