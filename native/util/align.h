#pragma once

#include <cstdint>

namespace HyoutaUtils::IO {
struct File;
}

namespace HyoutaUtils {
inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
    const uint32_t rest = value % alignment;
    return rest == 0 ? value : (value + (alignment - rest));
}
inline uint64_t AlignUp(uint64_t value, uint64_t alignment) {
    const uint64_t rest = value % alignment;
    return rest == 0 ? value : (value + (alignment - rest));
}

bool AlignFile(HyoutaUtils::IO::File& file, uint64_t& position, uint64_t alignment);
} // namespace HyoutaUtils
