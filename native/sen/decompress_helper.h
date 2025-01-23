#pragma once

#include <optional>
#include <vector>

namespace SenLib {
std::optional<std::vector<char>> DecompressFromBuffer(const char* buffer, size_t length);
} // namespace SenLib
