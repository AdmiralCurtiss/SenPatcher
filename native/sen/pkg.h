#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"

namespace SenLib {
struct PkgFile {
    // individual file unpacking logic can be found at 0x41ad80 in CS2

    std::array<char, 0x40> Filename;
    uint32_t UncompressedSize;
    uint32_t CompressedSize;
    uint32_t DataPosition;
    uint32_t Flags;

    // Pointer to the data stream. Must contain at least CompressedSize bytes!
    const char* Data = nullptr;

    PkgFile(SenLib::ReadStream& s, HyoutaUtils::EndianUtils::Endianness e);
};

struct Pkg {
    uint32_t Unknown; // seems like a unique index or something, at least in CS1/2?
    std::vector<PkgFile> Files;

    Pkg(const char* buffer,
        size_t length,
        HyoutaUtils::EndianUtils::Endianness e =
            HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    void WriteToStream(SenLib::WriteStream& s, HyoutaUtils::EndianUtils::Endianness e) const;
};
} // namespace SenLib
