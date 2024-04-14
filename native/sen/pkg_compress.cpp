#include "pkg_compress.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "lz4/lz4.h"
#include "lz4/lz4hc.h"

#include "crc32.h"
#include "sen/pkg.h"
#include "util/memwrite.h"

namespace SenLib {
bool CompressPkgFile(std::unique_ptr<char[]>& dataBuffer,
                     PkgFile& pkgFile,
                     const char* uncompressedData,
                     uint32_t uncompressedLength,
                     uint32_t flags,
                     HyoutaUtils::EndianUtils::Endianness e) {
    const bool wantsChecksum = (flags & static_cast<uint32_t>(0x2)) != 0;
    uint32_t checksum = 0;
    if (wantsChecksum) {
        crc_t crc = static_cast<crc_t>(uncompressedLength);
        crc = crc_update(crc, uncompressedData, uncompressedLength);
        checksum = static_cast<uint32_t>(crc);
    }

    std::unique_ptr<char[]> data;
    uint32_t length = 0;
    const uint32_t offset = wantsChecksum ? 4 : 0;
    uint32_t resultFlags = 0;

    const auto write_uncompressed = [&]() -> bool {
        if (uncompressedLength > UINT32_MAX - offset) {
            return false;
        }
        length = uncompressedLength + offset;
        data = std::make_unique_for_overwrite<char[]>(length);
        if (!data) {
            return false;
        }
        std::memcpy(data.get() + offset, uncompressedData, uncompressedLength);
        resultFlags = wantsChecksum ? 2u : 0u;
        return true;
    };

    if (flags & static_cast<uint32_t>(0x1)) {
        // this is the built-in compression from Falcom
        // TODO: implement
        return false;
    } else if (flags & static_cast<uint32_t>(0x4)) {
        // LZ4
        if (uncompressedLength == 0 || uncompressedLength > LZ4_MAX_INPUT_SIZE) {
            if (!write_uncompressed()) {
                return false;
            }
        } else {
            const int signedSize = static_cast<int>(uncompressedLength);
            const int bound = LZ4_compressBound(signedSize);
            if (bound <= 0) {
                if (!write_uncompressed()) {
                    return false;
                }
            } else {
                auto compressedData = std::make_unique_for_overwrite<char[]>(
                    static_cast<size_t>(static_cast<unsigned int>(bound)) + offset);
                if (!compressedData) {
                    return false;
                }
                const int lz4return = LZ4_compress_HC(uncompressedData,
                                                      compressedData.get() + offset,
                                                      signedSize,
                                                      bound,
                                                      LZ4HC_CLEVEL_MAX);
                if (lz4return <= 0 || static_cast<unsigned int>(lz4return) >= uncompressedLength) {
                    // compression failed or pointless, write uncompressed instead
                    compressedData.reset();
                    if (!write_uncompressed()) {
                        return false;
                    }
                } else {
                    length = static_cast<unsigned int>(lz4return) + offset;
                    data = std::move(compressedData);
                    resultFlags = (wantsChecksum ? 2u : 0u) | 4u;
                }
            }
        }
    } else if (flags & static_cast<uint32_t>(0x8)) {
        // LZMA
        // TODO: implement
        return false;
    } else if (flags & static_cast<uint32_t>(0x10)) {
        // ZSTD
        // TODO: implement
        return false;
    } else {
        // no compression
        if (!write_uncompressed()) {
            return false;
        }
    }

    if (wantsChecksum) {
        HyoutaUtils::MemWrite::WriteUInt32(data.get(),
                                           HyoutaUtils::EndianUtils::ToEndian(checksum, e));
    }

    pkgFile.UncompressedSize = uncompressedLength;
    pkgFile.CompressedSize = length;
    pkgFile.Flags = flags;
    pkgFile.Data = data.get();
    dataBuffer = std::move(data);

    return true;
}
} // namespace SenLib
