#pragma once

#include <array>
#include <cstring>
#include <string_view>

namespace SenPatcher {
consteval std::array<char, 0x100> InitializeP3AFilename(std::string_view path) {
    std::array<char, 0x100> filename;
    if (path.size() > filename.size()) {
        throw -1;
    }
    filename.fill('\0');
    for (size_t i = 0; i < path.size(); ++i) {
        filename[i] = path[i];
    }
    return filename;
}

bool CopyToP3AFilename(std::array<char, 0x100>& filename, std::string_view path);
} // namespace SenPatcher
