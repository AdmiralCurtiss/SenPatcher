#pragma once

#include <array>
#include <cstring>
#include <string_view>

namespace SenPatcher {
bool CopyToP3AFilename(std::array<char, 0x100>& filename, std::string_view path) {
    if (path.size() > filename.size()) {
        return false;
    }
    filename.fill('\0');
    std::memcpy(filename.data(), path.data(), path.size());
    return true;
}
} // namespace SenPatcher
