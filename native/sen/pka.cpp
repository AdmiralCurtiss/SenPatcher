#include "pka.h"

#include <array>
#include <cstring>

#include "util/endian.h"
#include "util/memread.h"

namespace SenLib {
PkaHeader::PkaHeader() = default;
PkaHeader::PkaHeader(PkaHeader&& other) = default;
PkaHeader& PkaHeader::operator=(PkaHeader&& other) = default;
PkaHeader::~PkaHeader() = default;

bool ReadPkaFromFile(PkaHeader& pka,
                     SenPatcher::IO::File& file,
                     HyoutaUtils::EndianUtils::Endianness e) {
    std::optional<uint64_t> pos = file.GetPosition();
    if (!pos.has_value()) {
        return false;
    }

    // This format unfortunately makes it rather difficult to determine the size of the header
    // without manually parsing most of it, so we need to do that...
    // TODO: Might be better to heuristically read a ~10 MB or whatever block of data and then
    // testing if that is enough, and increasing if it's not etc.

    std::array<uint32_t, 2> header1;
    if (file.Read(header1.data(), sizeof(header1)) != sizeof(header1)) {
        return false;
    }

    if (HyoutaUtils::EndianUtils::FromEndian(header1[0],
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)
        != 0x7ff7cf0d) {
        return false;
    }

    uint64_t headerLength = 8;
    const uint32_t pkgCount = HyoutaUtils::EndianUtils::FromEndian(header1[1], e);
    for (size_t i = 0; i < pkgCount; ++i) {
        if (!file.SetPosition(0x20, SenPatcher::IO::SetPositionMode::Current)) {
            return false;
        }
        uint32_t pkgFileCount;
        if (file.Read(&pkgFileCount, sizeof(pkgFileCount)) != sizeof(pkgFileCount)) {
            return false;
        }
        int64_t offset =
            static_cast<int64_t>(HyoutaUtils::EndianUtils::FromEndian(pkgFileCount, e)) * 0x60;
        if (!file.SetPosition(offset, SenPatcher::IO::SetPositionMode::Current)) {
            return false;
        }

        headerLength += (0x20 + 4 + static_cast<uint64_t>(offset));
    }

    uint32_t pkaFileCount;
    if (file.Read(&pkaFileCount, sizeof(pkaFileCount)) != sizeof(pkaFileCount)) {
        return false;
    }

    const uint64_t pkaFileLength =
        static_cast<uint64_t>(HyoutaUtils::EndianUtils::FromEndian(pkaFileCount, e)) * 0x34;
    headerLength += (4 + pkaFileLength);

    if (!file.SetPosition(*pos)) {
        return false;
    }

    auto buffer = std::make_unique_for_overwrite<char[]>(headerLength);
    if (file.Read(buffer.get(), headerLength) != headerLength) {
        return false;
    }

    return ReadPkaFromMemory(pka, buffer.get(), headerLength, e);
}

bool ReadPkaFromMemory(PkaHeader& pka,
                       const char* data,
                       size_t length,
                       HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;

    if (length < 8) {
        return false;
    }
    if (FromEndian(ReadUInt32(&data[0]), HyoutaUtils::EndianUtils::Endianness::LittleEndian)
        != 0x7ff7cf0d) {
        return false;
    }

    const uint32_t pkgCount = FromEndian(ReadUInt32(&data[4]), e);
    uint32_t totalPkgFileCount = 0;

    size_t offset = 8;
    for (size_t i = 0; i < pkgCount; ++i) {
        offset += 0x20;
        if (length - 4 < offset) {
            return false;
        }
        const uint32_t pkgFileCount = FromEndian(ReadUInt32(&data[offset]), e);
        offset += (4 + ((0x40 + 0x20) * pkgFileCount));
        totalPkgFileCount += pkgFileCount;
    }
    if (length - 4 < offset) {
        return false;
    }
    const uint32_t pkaFileCount = FromEndian(ReadUInt32(&data[offset]), e);

    auto pkgs = std::make_unique<PkaPkgToHashData[]>(pkgCount);
    auto pkgFiles = std::make_unique<PkaFileHashData[]>(totalPkgFileCount);
    auto files = std::make_unique<PkaHashToFileData[]>(pkaFileCount);
    uint32_t totalPkgFileOffset = 0;

    offset = 8;
    for (size_t i = 0; i < pkgCount; ++i) {
        auto& pkg = pkgs[i];
        std::memcpy(pkg.PkgName.data(), &data[offset], pkg.PkgName.size());
        offset += pkg.PkgName.size();
        const uint32_t pkgFileCount = FromEndian(ReadUInt32(&data[offset]), e);
        offset += 4;
        pkg.FileOffset = totalPkgFileOffset;
        pkg.FileCount = pkgFileCount;
        for (size_t j = 0; j < pkgFileCount; ++j) {
            PkaFileHashData& fhd = pkgFiles[totalPkgFileOffset + j];
            std::memcpy(fhd.Filename.data(), &data[offset], fhd.Filename.size());
            offset += fhd.Filename.size();
            std::memcpy(fhd.Hash.data(), &data[offset], fhd.Hash.size());
            offset += fhd.Hash.size();
        }
        totalPkgFileOffset += pkgFileCount;
    }

    offset += 4;
    for (size_t i = 0; i < pkaFileCount; ++i) {
        PkaHashToFileData& pkaFile = files[i];
        if (length - (pkaFile.Hash.size() + 8 + 4 + 4 + 4) < offset) {
            return false;
        }

        std::memcpy(pkaFile.Hash.data(), &data[offset], pkaFile.Hash.size());
        offset += pkaFile.Hash.size();
        pkaFile.Offset = FromEndian(ReadUInt64(&data[offset]), e);
        offset += 8;
        pkaFile.CompressedSize = FromEndian(ReadUInt32(&data[offset]), e);
        offset += 4;
        pkaFile.UncompressedSize = FromEndian(ReadUInt32(&data[offset]), e);
        offset += 4;
        pkaFile.Flags = FromEndian(ReadUInt32(&data[offset]), e);
        offset += 4;
    }

    pka.Pkgs = std::move(pkgs);
    pka.PkgCount = pkgCount;
    pka.PkgFiles = std::move(pkgFiles);
    pka.PkgFilesCount = totalPkgFileCount;
    pka.Files = std::move(files);
    pka.FilesCount = pkaFileCount;

    return true;
}

const PkaHashToFileData* FindFileInPkaByHash(const PkaHashToFileData* files,
                                             size_t length,
                                             const std::array<char, 0x20>& hash) {
    const PkaHashToFileData* infos = files;
    size_t count = length;
    while (true) {
        if (count == 0) {
            return nullptr;
        }

        const size_t countHalf = count / 2;
        const PkaHashToFileData* middle = infos + countHalf;
        const int cmp = memcmp(middle->Hash.data(), hash.data(), hash.size());
        if (cmp < 0) {
            infos = middle + 1;
            count = count - (countHalf + 1);
        } else if (cmp > 0) {
            count = countHalf;
        } else {
            return middle;
        }
    }
}
} // namespace SenLib
