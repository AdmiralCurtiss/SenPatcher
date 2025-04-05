#include "patch_pkg.h"

#include <array>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "modload/loaded_pka.h"
#include "p3a/pack.h"
#include "p3a/structs.h"
#include "p3a/util.h"
#include "sen/file_getter.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/bps.h"
#include "util/endian.h"
#include "util/hash/sha1.h"
#include "util/stream.h"

namespace SenLib::TX {
std::vector<char> PatchSingleFileInPkg(const char* file,
                                       size_t fileLength,
                                       const char* patch,
                                       size_t patchLength,
                                       size_t expectedFileCount,
                                       size_t fileToPatch) {
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

bool TryApplyPkgPatches(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
                        std::vector<SenPatcher::P3APackFile>& result,
                        std::string_view inpath,
                        std::string_view outpath,
                        size_t realsize,
                        const HyoutaUtils::Hash::SHA1& realhash,
                        size_t expectedFileCount,
                        std::span<const SingleFilePatchInfo> patches) {
    try {
        std::optional<SenPatcher::CheckedFileResult> file =
            getCheckedFile(inpath, realsize, realhash);
        if (!file) {
            return false;
        }

        std::vector<char> bin = std::move(file->Data);
        for (const SingleFilePatchInfo& patch : patches) {
            auto tex = PatchSingleFileInPkg(bin.data(),
                                            bin.size(),
                                            patch.PatchData,
                                            patch.PatchLength,
                                            expectedFileCount,
                                            patch.FileIndexToPatch);
            bin = std::move(tex);
        }


        // recompress remaining type1-compressed files as LZ4
        {
            PkgHeader pkg;
            if (!SenLib::ReadPkgFromMemory(pkg,
                                           bin.data(),
                                           bin.size(),
                                           HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                return false;
            }
            std::vector<std::unique_ptr<char[]>> recompressedData;
            recompressedData.reserve(pkg.FileCount);
            for (size_t i = 0; i < pkg.FileCount; ++i) {
                SenLib::PkgFile& f1 = pkg.Files[i];
                if (!f1.Data) {
                    return false;
                }
                uint32_t flags = f1.Flags;
                if (flags == 0x4) {
                    continue;
                }

                auto decompressedOriginal =
                    std::make_unique_for_overwrite<char[]>(f1.UncompressedSize);
                uint32_t compressedSize = f1.CompressedSize;
                const char* compressedData = f1.Data;

                if (!SenLib::ExtractAndDecompressPkgFile(
                        decompressedOriginal.get(),
                        f1.UncompressedSize,
                        compressedData,
                        compressedSize,
                        flags,
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                    return false;
                }
                if (!CompressPkgFile(recompressedData.emplace_back(),
                                     f1,
                                     decompressedOriginal.get(),
                                     f1.UncompressedSize,
                                     0x4,
                                     HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                    return false;
                }
            }

            std::vector<char> ms;
            {
                std::unique_ptr<char[]> buffer;
                size_t bufferLength;
                if (!SenLib::CreatePkgInMemory(
                        buffer,
                        bufferLength,
                        pkg,
                        HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                    return false;
                }
                ms.assign(buffer.get(), buffer.get() + bufferLength);
            }

            bin = std::move(ms);
        }


        std::array<char, 0x100> p3aFileName;
        if (!SenPatcher::CopyToP3AFilename(p3aFileName, outpath)) {
            return false;
        }
        result.emplace_back(std::move(bin), p3aFileName, SenPatcher::P3ACompressionType::None);

        return true;
    } catch (...) {
        return false;
    }
}
} // namespace SenLib::TX
