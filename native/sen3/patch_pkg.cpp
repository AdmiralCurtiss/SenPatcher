#include "patch_pkg.h"

#include <memory>
#include <vector>

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
    if (!SenLib::ExtractAndDecompressPkgFile(decompressedOriginal.get(),
                                             f1.UncompressedSize,
                                             f1.Data,
                                             f1.CompressedSize,
                                             f1.Flags,
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
