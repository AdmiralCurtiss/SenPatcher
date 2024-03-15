#pragma once

#include <cstdint>

namespace SenPatcher {
struct Logger;
}

namespace SenLib::Sen4 {
enum class GameVersion {
    Unknown,
    English,
    Japanese,
};

inline char* GetCodeAddressJpEn(GameVersion version,
                                char* textRegion,
                                uint64_t addressJp,
                                uint64_t addressEn) {
    return textRegion
           + (version == GameVersion::Japanese ? (addressJp - 0x140001000u)
                                               : (addressEn - 0x140001000u));
}
} // namespace SenLib::Sen4
