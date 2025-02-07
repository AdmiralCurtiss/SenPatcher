#pragma once

#include <optional>
#include <vector>

namespace SenLib {
std::optional<std::vector<char>> DecompressFromBuffer(const char* buffer, size_t length);

struct DecompressHelperAlignedBuffer {
    std::vector<char> UnalignedData;
    char* AlignedData;
    size_t Length;
};
std::optional<DecompressHelperAlignedBuffer>
    AlignedDecompressFromBuffer(const char* buffer, size_t length, size_t alignment);
} // namespace SenLib
