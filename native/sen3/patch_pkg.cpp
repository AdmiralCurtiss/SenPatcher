#include "patch_pkg.h"

#include <memory>
#include <vector>

#include "util/bps.h"
#include "lz4/lz4.h"
#include "sen/pkg.h"
#include "util/stream.h"

namespace SenLib::Sen3 {
std::vector<char> PatchSingleTexturePkg(const char* file,
                                        size_t fileLength,
                                        const char* patch,
                                        size_t patchLength) {
    PkgHeader pkg;

    if (!SenLib::ReadPkgFromMemory(
            pkg, file, fileLength, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        throw "failed to read pkg";
    }

    if (pkg.FileCount < 2 || !pkg.Files[1].Data) {
        throw "invalid single texture pkg";
    }
    auto& f1 = pkg.Files[1];
    auto decompressedOriginal = std::make_unique<char[]>(f1.UncompressedSize);
    if (LZ4_decompress_safe(
            f1.Data, decompressedOriginal.get(), f1.CompressedSize, f1.UncompressedSize)
        != f1.UncompressedSize) {
        throw "decompression error";
    }

    SenLib::DuplicatableByteArrayStream originalStream(decompressedOriginal.get(),
                                                       f1.UncompressedSize);
    SenLib::DuplicatableByteArrayStream patchStream(patch, patchLength);
    std::vector<char> modified;
    HyoutaUtils::Bps::ApplyPatchToStream(originalStream, patchStream, modified);

    std::vector<char> modifiedCompressed;
    modifiedCompressed.resize(LZ4_compressBound(modified.size()));
    int modifiedCompressedSize = LZ4_compress_default(
        modified.data(), modifiedCompressed.data(), modified.size(), modifiedCompressed.size());
    if (modifiedCompressedSize <= 0) {
        throw "compression error";
    }

    f1.Data = modifiedCompressed.data();
    f1.UncompressedSize = modified.size();
    f1.CompressedSize = modifiedCompressedSize;

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
