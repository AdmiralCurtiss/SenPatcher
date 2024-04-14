#include "pkg.h"

#include "util/endian.h"
#include "util/memread.h"
#include "util/memwrite.h"

namespace SenLib {
PkgHeader::PkgHeader() = default;
PkgHeader::PkgHeader(PkgHeader&& other) = default;
PkgHeader& PkgHeader::operator=(PkgHeader&& other) = default;
PkgHeader::~PkgHeader() = default;

bool ReadPkgFromMemory(PkgHeader& pkg,
                       const char* buffer,
                       size_t length,
                       HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;

    if (length < 8) {
        return false;
    }

    const uint32_t unknown = FromEndian(ReadUInt32(&buffer[0]), e);
    const uint32_t fileCount = FromEndian(ReadUInt32(&buffer[4]), e);
    const size_t headerSize = 8 + (static_cast<size_t>(pkg.FileCount) * 0x50);
    if (length < headerSize) {
        return false;
    }

    auto files = std::make_unique<PkgFile[]>(fileCount);
    size_t pos = 8;
    for (size_t i = 0; i < fileCount; ++i) {
        auto& f = files[i];

        f.Filename = ReadArray<0x40>(&buffer[pos]);
        f.UncompressedSize = FromEndian(ReadUInt32(&buffer[pos + 0x40]), e);
        f.CompressedSize = FromEndian(ReadUInt32(&buffer[pos + 0x44]), e);
        f.DataPosition = FromEndian(ReadUInt32(&buffer[pos + 0x48]), e);
        f.Flags = FromEndian(ReadUInt32(&buffer[pos + 0x4c]), e);

        if (f.CompressedSize < length && f.DataPosition <= length - f.CompressedSize) {
            f.Data = buffer + f.DataPosition;
        } else {
            f.Data = nullptr;
        }

        pos += 0x50;
    }

    pkg.Files = std::move(files);
    pkg.FileCount = fileCount;
    pkg.Unknown = unknown;

    return true;
}

bool CreatePkgInMemory(std::unique_ptr<char[]>& buffer,
                       size_t& bufferLength,
                       const PkgHeader& pkg,
                       HyoutaUtils::EndianUtils::Endianness e) {
    return CreatePkgInMemory(buffer, bufferLength, pkg.Files.get(), pkg.FileCount, pkg.Unknown, e);
}

bool CreatePkgInMemory(std::unique_ptr<char[]>& buffer,
                       size_t& bufferLength,
                       const PkgFile* pkgFiles,
                       uint32_t pkgFileCount,
                       uint32_t unknownValue,
                       HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::ToEndian;

    const size_t headerSize = 8 + (static_cast<size_t>(pkgFileCount) * 0x50);
    size_t totalFileSize = 0;
    for (size_t i = 0; i < pkgFileCount; ++i) {
        const auto& f = pkgFiles[i];
        if (!f.Data) {
            return false;
        }

        totalFileSize += f.CompressedSize;
    }

    const size_t dataLength = headerSize + totalFileSize;
    auto data = std::make_unique_for_overwrite<char[]>(dataLength);
    size_t currentFileSize = 0;

    WriteUInt32(&data[0], ToEndian(unknownValue, e));
    WriteUInt32(&data[4], ToEndian(pkgFileCount, e));
    size_t pos = 8;
    for (size_t i = 0; i < pkgFileCount; ++i) {
        const auto& f = pkgFiles[i];
        WriteArray(&data[pos], f.Filename);
        WriteUInt32(&data[pos + 0x40], ToEndian(f.UncompressedSize, e));
        WriteUInt32(&data[pos + 0x44], ToEndian(f.CompressedSize, e));
        WriteUInt32(&data[pos + 0x48],
                    ToEndian(static_cast<uint32_t>(headerSize + currentFileSize), e));
        WriteUInt32(&data[pos + 0x4c], ToEndian(f.Flags, e));

        pos += 0x50;
        currentFileSize += f.CompressedSize;
    }

    for (size_t i = 0; i < pkgFileCount; ++i) {
        auto& f = pkgFiles[i];
        std::memcpy(&data[pos], f.Data, f.CompressedSize);
        pos += f.CompressedSize;
    }

    buffer = std::move(data);
    bufferLength = dataLength;

    return true;
}
} // namespace SenLib
