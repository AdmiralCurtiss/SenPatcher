#pragma once

#include <cstdint>
#include <optional>

namespace HyoutaUtils::Hash {
constexpr std::optional<uint8_t> ParseHexDigit(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>((c - 'a') + 10);
    } else if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>((c - 'A') + 10);
    } else {
        return std::nullopt; // invalid hex digit
    }
}
} // namespace HyoutaUtils::Hash
