#pragma once

#include <cstdint>

namespace SenPatcher::IO {
struct File;
}

namespace SenPatcher {
inline uint64_t AlignUp(uint64_t value, uint64_t alignment) {
    const uint64_t rest = value % alignment;
    return rest == 0 ? value : (value + (alignment - rest));
}

bool AlignFile(SenPatcher::IO::File& file, uint64_t& position, uint64_t alignment);
} // namespace SenPatcher
