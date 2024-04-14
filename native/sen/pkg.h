#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "util/endian.h"

namespace SenLib {
struct PkgFile {
    // individual file unpacking logic can be found at 0x41ad80 in CS2

    std::array<char, 0x40> Filename;
    uint32_t UncompressedSize;
    uint32_t CompressedSize;
    uint32_t DataPosition;
    uint32_t Flags;

    // Pointer to the data stream. Must contain at least CompressedSize bytes!
    const char* Data;
};

struct PkgHeader {
    std::unique_ptr<PkgFile[]> Files;
    uint32_t FileCount = 0;
    uint32_t Unknown = 0; // seems like a unique index or something, at least in CS1/2?

    PkgHeader();
    PkgHeader(const PkgHeader& other) = delete;
    PkgHeader(PkgHeader&& other);
    PkgHeader& operator=(const PkgHeader& other) = delete;
    PkgHeader& operator=(PkgHeader&& other);
    ~PkgHeader();
};

bool ReadPkgFromMemory(
    PkgHeader& pkg,
    const char* buffer,
    size_t length,
    HyoutaUtils::EndianUtils::Endianness e = HyoutaUtils::EndianUtils::Endianness::LittleEndian);

bool CreatePkgInMemory(std::unique_ptr<char[]>& buffer,
                       size_t& bufferLength,
                       const PkgHeader& pkg,
                       HyoutaUtils::EndianUtils::Endianness e);
bool CreatePkgInMemory(std::unique_ptr<char[]>& buffer,
                       size_t& bufferLength,
                       const PkgFile* pkgFiles,
                       uint32_t pkgFileCount,
                       uint32_t unknownValue,
                       HyoutaUtils::EndianUtils::Endianness e);

} // namespace SenLib
