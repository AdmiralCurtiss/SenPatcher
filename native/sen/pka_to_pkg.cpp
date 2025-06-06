#include "pka_to_pkg.h"

#include <memory>

#include "sen/pka.h"
#include "sen/pkg.h"
#include "util/file.h"

namespace SenLib {
bool ConvertPkaToSinglePkg(PkgHeader& pkg,
                           std::unique_ptr<char[]>& dataBuffer,
                           const PkaHeader& pka,
                           size_t index,
                           HyoutaUtils::IO::File& file,
                           std::vector<SenLib::ReferencedPka>& referencedPkas,
                           bool convertAsPkaReferenceStub) {
    if (index >= pka.PkgCount) {
        return false;
    }

    const PkaPkgToHashData& pkaPkgToHashData = pka.Pkgs[index];
    const uint32_t fileCount = pkaPkgToHashData.FileCount;
    if (pkaPkgToHashData.FileOffset >= pka.PkgFilesCount) {
        return false;
    }
    if (fileCount > (pka.PkgFilesCount - pkaPkgToHashData.FileOffset)) {
        return false;
    }
    const PkaFileHashData* const pkaFiles = &pka.PkgFiles[pkaPkgToHashData.FileOffset];
    size_t headerSize = 8 + (static_cast<size_t>(fileCount) * 0x50);
    size_t requiredMemorySize = 0;
    if (convertAsPkaReferenceStub) {
        requiredMemorySize = (static_cast<size_t>(0x20) * fileCount);
    } else {
        for (size_t i = 0; i < fileCount; ++i) {
            auto* f = FindFileInPkaByHash(pka.Files.get(), pka.FilesCount, pkaFiles[i].Hash);
            if (!f) {
                for (auto& refPka : referencedPkas) {
                    f = FindFileInPkaByHash(refPka.PkaHeader.Files.get(),
                                            refPka.PkaHeader.FilesCount,
                                            pkaFiles[i].Hash);
                    if (f) {
                        break;
                    }
                }
            }
            if (!f) {
                return false;
            }

            requiredMemorySize += f->CompressedSize;
        }
    }

    auto pkgFiles = std::make_unique<PkgFile[]>(fileCount);
    auto data = std::make_unique_for_overwrite<char[]>(requiredMemorySize);
    size_t dataOffset = 0;
    for (size_t i = 0; i < fileCount; ++i) {
        auto* f = FindFileInPkaByHash(pka.Files.get(), pka.FilesCount, pkaFiles[i].Hash);
        HyoutaUtils::IO::File* hf = &file;
        if (!f) {
            for (auto& refPka : referencedPkas) {
                f = FindFileInPkaByHash(
                    refPka.PkaHeader.Files.get(), refPka.PkaHeader.FilesCount, pkaFiles[i].Hash);
                if (f) {
                    hf = &refPka.PkaFile;
                    break;
                }
            }
        }
        if (!f) {
            return false;
        }

        pkgFiles[i].Filename = pkaFiles[i].Filename;
        pkgFiles[i].UncompressedSize = f->UncompressedSize;
        pkgFiles[i].CompressedSize = f->CompressedSize;
        pkgFiles[i].DataPosition = static_cast<uint32_t>(headerSize + dataOffset);
        pkgFiles[i].Flags = f->Flags;
        pkgFiles[i].Data = &data[dataOffset];

        if (convertAsPkaReferenceStub) {
            std::memcpy(&data[dataOffset], f->Hash.data(), f->Hash.size());
            pkgFiles[i].CompressedSize = static_cast<uint32_t>(f->Hash.size());
            pkgFiles[i].Flags = 0x80;
            dataOffset += f->Hash.size();
        } else {
            if (!hf->SetPosition(f->Offset)) {
                return false;
            }
            if (hf->Read(&data[dataOffset], f->CompressedSize) != f->CompressedSize) {
                return false;
            }
            dataOffset += f->CompressedSize;
        }
    }

    dataBuffer = std::move(data);
    pkg.Files = std::move(pkgFiles);
    pkg.FileCount = fileCount;
    pkg.Unknown = 0;

    return true;
}
} // namespace SenLib
