#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace SenTools {
struct RomMapperRegion {
    uint64_t RomStart;
    uint64_t RamStart;
    uint64_t Length;
};

struct RomMapper {
    std::unique_ptr<RomMapperRegion[]> Regions;
    size_t NumberOfRegions;

    std::optional<uint64_t> MapRamToRom(uint64_t ramAddress) const;
    std::optional<uint64_t> MapRomToRam(uint64_t romAddress) const;
};
}; // namespace SenTools
