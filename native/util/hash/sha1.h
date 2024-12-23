#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include "util/file.h"
#include "util/hash/util.h"

namespace HyoutaUtils::Hash {
struct SHA1 {
    std::array<char, 20> Hash;
};
static_assert(sizeof(SHA1) == 20);

SHA1 CalculateSHA1(const void* data, size_t length) noexcept;
std::optional<SHA1> CalculateSHA1FromFile(HyoutaUtils::IO::File& file) noexcept;

constexpr std::optional<SHA1> TrySHA1FromHexString(std::string_view sv) {
    if (sv.size() != 40) {
        return std::nullopt; // invalid sha1 hex string
    }

    SHA1 result;
    for (size_t i = 0; i < 20; ++i) {
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

consteval SHA1 SHA1FromHexString(std::string_view sv) {
    return TrySHA1FromHexString(sv).value();
}

inline bool operator==(const SHA1& lhs, const SHA1& rhs) {
    return lhs.Hash == rhs.Hash;
}

inline bool operator!=(const SHA1& lhs, const SHA1& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::Hash
