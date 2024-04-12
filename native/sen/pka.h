#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"

#include "file.h"

namespace SenLib {
struct PkaFileHashData {
    std::array<char, 0x40> Filename;
    std::array<char, 0x20> Hash;
};

struct PkaPkgToHashData {
    std::array<char, 0x20> PkgName;
    uint32_t FileOffset; // index to first file in PkgFiles in header
    uint32_t FileCount;
};

struct PkaHashToFileData {
    std::array<char, 0x20> Hash;
    uint64_t Offset;
    uint32_t CompressedSize;
    uint32_t UncompressedSize;
    uint32_t Flags;
};

struct PkaHeader {
    std::unique_ptr<PkaPkgToHashData[]> Pkgs;
    size_t PkgCount = 0;
    std::unique_ptr<PkaFileHashData[]> PkgFiles;
    size_t PkgFilesCount = 0;
    std::unique_ptr<PkaHashToFileData[]> Files;
    size_t FilesCount = 0;

    PkaHeader();
    PkaHeader(const PkaHeader& other) = delete;
    PkaHeader(PkaHeader&& other);
    PkaHeader& operator=(const PkaHeader& other) = delete;
    PkaHeader& operator=(PkaHeader&& other);
    ~PkaHeader();
};

bool ReadPkaFromFile(
    PkaHeader& pka,
    SenPatcher::IO::File& file,
    HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian);
bool ReadPkaFromMemory(
    PkaHeader& pka,
    const char* data,
    size_t length,
    HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian);
const PkaHashToFileData* FindFileInPkaByHash(const PkaHashToFileData* files,
                                             size_t length,
                                             const std::array<char, 0x20>& hash);
} // namespace SenLib
