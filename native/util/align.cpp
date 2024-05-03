#include "align.h"

#include <array>
#include <cstdint>

#include "util/file.h"

namespace HyoutaUtils {
bool AlignFile(HyoutaUtils::IO::File& file, uint64_t& position, uint64_t alignment) {
    const uint64_t newPos = AlignUp(position, alignment);
    uint64_t nullBytesToWrite = newPos - position;
    if (nullBytesToWrite == 0) {
        return true;
    }

    position += nullBytesToWrite;
    std::array<char, 0x100> nulls{};
    do {
        const uint64_t w = nullBytesToWrite > nulls.size() ? nulls.size() : nullBytesToWrite;
        if (file.Write(nulls.data(), w) != w) {
            return false;
        }
        nullBytesToWrite -= w;
    } while (nullBytesToWrite != 0);

    return true;
}
} // namespace HyoutaUtils
