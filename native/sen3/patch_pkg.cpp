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
#include "sen/pka.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/bps.h"
#include "util/endian.h"
#include "util/hash/sha1.h"
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

bool TryApplyPkgPatches(const SenPatcher::GetCheckedFileCallback& getCheckedFile,
                        std::vector<SenPatcher::P3APackFile>& result,
                        SenLib::ModLoad::LoadedPkaData& vanillaPKAs,
                        const SenPatcher::GetCheckedFileCallback& getCheckedFilePkaStub,
                        std::string_view inpath,
                        std::string_view outpath,
                        size_t stubsize,
                        const HyoutaUtils::Hash::SHA1& stubhash,
                        size_t realsize,
                        const HyoutaUtils::Hash::SHA1& realhash,
                        size_t expectedFileCount,
                        std::span<const SingleFilePatchInfo> patches) {
    try {
        // first try to get this as a pka-referencing pkg
        std::optional<SenPatcher::CheckedFileResult> file =
            getCheckedFilePkaStub(inpath, stubsize, stubhash);
        if (!file) {
            // try the full pkg instead, this is less optimal filesize-wise but more compatible
            // since it also works if eg. the user has extracted the .pka and only has the .pkgs
            // lying around for modding purposes
            file = getCheckedFile(inpath, realsize, realhash);
        }
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
                                            patch.FileIndexToPatch,
                                            &vanillaPKAs);
            bin = std::move(tex);
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
} // namespace SenLib::Sen3
