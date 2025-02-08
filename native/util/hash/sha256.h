#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include "util/hash/util.h"

namespace HyoutaUtils::Hash {
struct SHA256 {
    std::array<char, 32> Hash;
};

SHA256 CalculateSHA256(const void* data, size_t length) noexcept;

constexpr std::optional<SHA256> TrySHA256FromHexString(std::string_view sv) {
    if (sv.size() != 64) {
        return std::nullopt; // invalid sha256 hex string
    }

    SHA256 result;
    for (size_t i = 0; i < 32; ++i) {
        const char c0 = sv[i * 2];
        const char c1 = sv[i * 2 + 1];
        const auto d0 = ParseHexDigit(c0);
        if (!d0) {
            return std::nullopt;
        }
        const auto d1 = ParseHexDigit(c1);
        if (!d1) {
            return std::nullopt;
        }
        result.Hash[i] = static_cast<char>(((*d0) << 4) | (*d1));
    }
    return result;
}

consteval SHA256 SHA256FromHexString(std::string_view sv) {
    return TrySHA256FromHexString(sv).value();
}

inline bool operator==(const SHA256& lhs, const SHA256& rhs) {
    return lhs.Hash == rhs.Hash;
}

inline bool operator!=(const SHA256& lhs, const SHA256& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::Hash
