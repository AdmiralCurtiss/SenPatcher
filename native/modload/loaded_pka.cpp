#include "loaded_pka.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "sen/pka.h"
#include "util/file.h"

namespace SenLib::ModLoad {
bool BuildFakePkaPkg(char* memory,
                     const SenLib::PkaPkgToHashData* pkaPkg,
                     SenLib::PkaFileHashData* pkgFiles,
                     SenLib::PkaHashToFileData* files,
                     size_t filesCount) {
    char* header = memory;
    std::memcpy(header, &pkaPkg->PkgName[pkaPkg->PkgName.size() - 4], 4);
    std::memcpy(header + 4, &pkaPkg->FileCount, 4);
    header += 8;
    char* data = header + (pkaPkg->FileCount * 0x50);
    uint32_t dataPosition = 8 + (pkaPkg->FileCount * 0x50);
    for (size_t i = 0; i < pkaPkg->FileCount; ++i) {
        const SenLib::PkaFileHashData& fileHashData = pkgFiles[pkaPkg->FileOffset + i];
        const SenLib::PkaHashToFileData* fileData =
            SenLib::FindFileInPkaByHash(files, filesCount, fileHashData.Hash);
        if (!fileData) {
            return false;
        }
        std::array<uint32_t, 4> headerData;
        headerData[0] = fileData->UncompressedSize;
        headerData[1] = 0x20; // compressed size, always the SHA256 hash
        headerData[2] = dataPosition;
        headerData[3] = 0x80; // fake flags to indicate that it should look in the PKA

        assert(fileHashData.Filename.size() == 0x40);
        assert(fileHashData.Hash.size() == 0x20);
        std::memcpy(header, fileHashData.Filename.data(), fileHashData.Filename.size());
        std::memcpy(header + 0x40, headerData.data(), 0x10);
        std::memcpy(data, fileHashData.Hash.data(), fileHashData.Hash.size());

        header += 0x50;
        data += 0x20;
        dataPosition += 0x20;
    }
    return true;
}

int32_t GetPkaPkgFilesize(SenLib::PkaPkgToHashData* pkgs,
                          size_t pkgCount,
                          const char* pkgPrefix,
                          size_t pkgPrefixLength,
                          const std::array<char, 0x100>& filteredPath,
                          const char* path,
                          uint32_t* out_filesize) {
    if (pkgCount > 0 && memcmp(pkgPrefix, filteredPath.data(), pkgPrefixLength) == 0) {
        // first check for the real PKG
        HyoutaUtils::IO::File file(std::string_view(path), HyoutaUtils::IO::OpenMode::Read);
        if (file.IsOpen()) {
            auto length = file.GetLength();
            if (!length) {
                return 0;
            }
            if (out_filesize) {
                *out_filesize = static_cast<uint32_t>(*length);
            }
            return 1;
        }

        // then check for data in the PKA
        const size_t start = pkgPrefixLength;
        assert(filteredPath.size() - start >= pkgs[0].PkgName.size());
        const SenLib::PkaPkgToHashData* pkaPkg =
            SenLib::FindPkgInPkaByName(pkgs, pkgCount, &filteredPath[start]);
        if (pkaPkg) {
            if (out_filesize) {
                // this pkg isn't actually real, but its size when crafted is going to be:
                // 8 bytes fixed header
                // 0x50 bytes header per file
                // 0x20 bytes data per file (the SHA256 hash)
                *out_filesize = 8 + (pkaPkg->FileCount * (0x50 + 0x20));
            }
            return 1;
        }
    }
    return -1;
}
} // namespace SenLib::ModLoad
