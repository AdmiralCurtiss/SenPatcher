#pragma once

namespace HyoutaUtils::Hash {
consteval char ParseHexDigit(char c) {
    if (c >= '0' && c <= '9') {
        return (c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return ((c - 'a') + 10);
    } else if (c >= 'A' && c <= 'F') {
        return ((c - 'A') + 10);
    } else {
        throw -1; // invalid hex digit
    }
}
} // namespace HyoutaUtils::Hash
