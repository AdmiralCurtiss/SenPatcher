#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include "util/hash/util.h"

namespace HyoutaUtils::Hash {
struct MD5 {
    std::array<char, 16> Hash;
};
static_assert(sizeof(MD5) == 16);

constexpr std::optional<MD5> TryMD5FromHexString(std::string_view sv) {
    if (sv.size() != 32) {
        return std::nullopt; // invalid sha1 hex string
    }

    MD5 result;
    for (size_t i = 0; i < 16; ++i) {
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

consteval MD5 MD5FromHexString(std::string_view sv) {
    return TryMD5FromHexString(sv).value();
}

inline bool operator==(const MD5& lhs, const MD5& rhs) {
    return lhs.Hash == rhs.Hash;
}

inline bool operator!=(const MD5& lhs, const MD5& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::Hash
