#include "loaded_pka.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "sen/pka.h"

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
} // namespace SenLib::ModLoad
