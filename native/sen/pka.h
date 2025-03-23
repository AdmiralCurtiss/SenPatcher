#pragma once

#include <array>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "util/endian.h"
#include "util/stream.h"

#include "util/file.h"

namespace SenLib {
struct PkaFileHashData {
    std::array<char, 0x40> Filename;
    std::array<char, 0x20> Hash;
};
static_assert(sizeof(PkaFileHashData) == 0x60);
static_assert(std::is_trivially_copyable_v<PkaFileHashData>);

struct PkaPkgToHashData {
    std::array<char, 0x20> PkgName;
    uint32_t FileOffset; // index to first file in PkgFiles in header
    uint32_t FileCount;
};
static_assert(sizeof(PkaPkgToHashData) == 0x28);
static_assert(std::is_trivially_copyable_v<PkaPkgToHashData>);

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

    PkaHeader() = default;
    PkaHeader(const PkaHeader& other) = delete;
    PkaHeader(PkaHeader&& other);
    PkaHeader& operator=(const PkaHeader& other) = delete;
    PkaHeader& operator=(PkaHeader&& other);
    ~PkaHeader();
};

bool ReadPkaFromFile(
    PkaHeader& pka,
    HyoutaUtils::IO::File& file,
    HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian);
bool ReadPkaFromMemory(
    PkaHeader& pka,
    const char* data,
    size_t length,
    HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian);
const PkaHashToFileData*
    FindFileInPkaByHash(const PkaHashToFileData* files, size_t length, const char* hash);
const PkaHashToFileData* FindFileInPkaByHash(const PkaHashToFileData* files,
                                             size_t length,
                                             const std::array<char, 0x20>& hash);
const PkaPkgToHashData*
    FindPkgInPkaByName(const PkaPkgToHashData* pkgs, size_t pkgCount, const char* pkgName);
} // namespace SenLib
