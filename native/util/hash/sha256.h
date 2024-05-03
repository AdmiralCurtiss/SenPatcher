#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "util/hash/util.h"

namespace HyoutaUtils::Hash {
struct SHA256 {
    std::array<char, 32> Hash;
};

SHA256 CalculateSHA256(void* data, size_t length);

consteval SHA256 SHA256FromHexString(std::string_view sv) {
    if (sv.size() != 64) {
        throw -1; // invalid sha256 hex string
    }

    SHA256 result;
    for (size_t i = 0; i < 32; ++i) {
        const char c0 = sv[i * 2];
        const char c1 = sv[i * 2 + 1];
        result.Hash[i] = (ParseHexDigit(c0) << 4) | ParseHexDigit(c1);
    }
    return result;
}

inline bool operator==(const SHA256& lhs, const SHA256& rhs) {
    return lhs.Hash == rhs.Hash;
}

inline bool operator!=(const SHA256& lhs, const SHA256& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::Hash
