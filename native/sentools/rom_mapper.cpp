#include "rom_mapper.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace SenTools {
std::optional<uint64_t> RomMapper::MapRamToRom(uint64_t ramAddress) const {
    for (size_t i = 0; i < NumberOfRegions; ++i) {
        const auto& r = Regions[i];
        if (r.RamStart <= ramAddress && ramAddress < (r.RamStart + r.Length)) {
            return ramAddress - (r.RamStart - r.RomStart);
        }
    }
    return std::nullopt;
}

std::optional<uint64_t> RomMapper::MapRomToRam(uint64_t romAddress) const {
    for (size_t i = 0; i < NumberOfRegions; ++i) {
        const auto& r = Regions[i];
        if (r.RomStart <= romAddress && romAddress < (r.RomStart + r.Length)) {
            return romAddress + (r.RamStart - r.RomStart);
        }
    }
    return std::nullopt;
}
}; // namespace SenTools
