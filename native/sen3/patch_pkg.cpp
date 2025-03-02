#include "patch_pkg.h"

#include <memory>
#include <mutex>
#include <vector>

#include "modload/loaded_pka.h"
#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/bps.h"
#include "util/endian.h"
#include "util/stream.h"

namespace SenLib::Sen3 {
std::vector<char> PatchSingleFileInPkg(const char* file,
                                       size_t fileLength,
                                       const char* patch,
                                       size_t patchLength,
                                       size_t expectedFileCount,
                                       size_t fileToPatch,
                                       SenLib::ModLoad::LoadedPkaData* vanillaPKAs) {
    PkgHeader pkg;

    if (!SenLib::ReadPkgFromMemory(
            pkg, file, fileLength, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        throw "failed to read pkg";
    }

    if (pkg.FileCount != expectedFileCount) {
        throw "invalid pkg";
    }
    for (size_t i = 0; i < pkg.FileCount; ++i) {
        if (!pkg.Files[i].Data) {
            throw "invalid pkg";
        }
    }

    SenLib::PkgFile& f1 = pkg.Files[fileToPatch];
    auto decompressedOriginal = std::make_unique_for_overwrite<char[]>(f1.UncompressedSize);
    uint32_t compressedSize = f1.CompressedSize;
    uint32_t flags = f1.Flags;
    const char* compressedData = f1.Data;
    std::unique_ptr<char[]> compressedDataBuffer = nullptr;
    if (vanillaPKAs && (flags & 0x80) && compressedSize == 0x20) {
        // this is a PKA hash, look up the data in the PKA
        const SenLib::PkaHashToFileData* fileData = SenLib::FindFileInPkaByHash(
            vanillaPKAs->Hashes.Files.get(), vanillaPKAs->Hashes.FilesCount, compressedData);
        if (fileData && fileData->UncompressedSize == f1.UncompressedSize) {
            const size_t index = static_cast<size_t>(static_cast<size_t>(fileData->Offset >> 48)
                                                     & static_cast<size_t>(0xffff));
            if (index < vanillaPKAs->HandleCount) {
                auto& pka = vanillaPKAs->Handles[index];
                std::lock_guard<std::recursive_mutex> lock(pka.Mutex);
                if (pka.Handle.SetPosition(fileData->Offset
                                           & static_cast<uint64_t>(0xffff'ffff'ffff))) {
                    compressedDataBuffer = std::make_unique<char[]>(fileData->CompressedSize);
                    if (compressedDataBuffer) {
                        if (pka.Handle.Read(compressedDataBuffer.get(), fileData->CompressedSize)
                            == fileData->CompressedSize) {
                            compressedData = compressedDataBuffer.get();
                            compressedSize = fileData->CompressedSize;
                            flags = fileData->Flags;
                        } else {
                            compressedDataBuffer.reset();
                        }
                    }
                }
            }
        }
    }

    if (!SenLib::ExtractAndDecompressPkgFile(decompressedOriginal.get(),
                                             f1.UncompressedSize,
                                             compressedData,
                                             compressedSize,
                                             flags,
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        throw "decompression error";
    }

    HyoutaUtils::Stream::DuplicatableByteArrayStream originalStream(decompressedOriginal.get(),
                                                                    f1.UncompressedSize);
    HyoutaUtils::Stream::DuplicatableByteArrayStream patchStream(patch, patchLength);
    std::vector<char> modified;
    HyoutaUtils::Bps::ApplyPatchToStream(originalStream, patchStream, modified);

    std::unique_ptr<char[]> modifiedCompressed;
    if (!CompressPkgFile(modifiedCompressed,
                         f1,
                         modified.data(),
                         modified.size(),
                         0x4,
                         HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        throw "compression error";
    }

    std::vector<char> ms;
    {
        std::unique_ptr<char[]> buffer;
        size_t bufferLength;
        if (!SenLib::CreatePkgInMemory(
                buffer, bufferLength, pkg, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            throw "pkg creation error";
        }
        ms.assign(buffer.get(), buffer.get() + bufferLength);
    }

    return ms;
}
} // namespace SenLib::Sen3
