#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "util/hash/util.h"

namespace HyoutaUtils::Hash {
struct SHA1 {
    std::array<char, 20> Hash;
};

SHA1 CalculateSHA1(const void* data, size_t length);

consteval SHA1 SHA1FromHexString(std::string_view sv) {
    if (sv.size() != 40) {
        throw -1; // invalid sha1 hex string
    }

    SHA1 result;
    for (size_t i = 0; i < 20; ++i) {
        const char c0 = sv[i * 2];
        const char c1 = sv[i * 2 + 1];
        result.Hash[i] = (ParseHexDigit(c0) << 4) | ParseHexDigit(c1);
    }
    return result;
}

inline bool operator==(const SHA1& lhs, const SHA1& rhs) {
    return lhs.Hash == rhs.Hash;
}

inline bool operator!=(const SHA1& lhs, const SHA1& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::Hash
