#include "pkg_extract.h"

#include <cstddef>
#include <cstring>
#include <memory>

#include "lz4/lz4.h"
#include "lzma2301/LzmaDec.h"
#include "zstd/zstd.h"

#include "util/hash/crc32.h"
#include "pkg.h"
#include "util/endian.h"
#include "util/memread.h"

namespace SenLib {
static bool DecompressType1(std::unique_ptr<char[]>& dataBuffer,
                            uint32_t uncompressedSize,
                            const char* compressedData,
                            uint32_t compressedSize,
                            HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;

    if (compressedSize < 12) {
        return false;
    }

    // decompression algorithm derived from 0x41aa50 in CS2
    // very simple, can only copy verbatim bytes or reference already written bytes
    auto target = std::make_unique_for_overwrite<char[]>(uncompressedSize);
    // the game ignores the target size here...
    const uint32_t targetSize = FromEndian(ReadUInt32(&compressedData[0]), e);
    const uint32_t sourceSize = FromEndian(ReadUInt32(&compressedData[4]), e);
    // this is in fact read and compared as 32-bit, very odd
    const uint32_t backrefByte = FromEndian(ReadUInt32(&compressedData[8]), e);
    uint32_t targetPosition = 0;
    uint32_t sourcePosition = 12;
    while (sourcePosition < sourceSize) {
        if (targetPosition >= uncompressedSize) {
            return false;
        }

        if (sourcePosition >= compressedSize) {
            return false;
        }
        const uint8_t sourceByte = static_cast<uint8_t>(compressedData[sourcePosition]);
        ++sourcePosition;
        if (sourceByte == backrefByte) {
            if (sourcePosition >= compressedSize) {
                return false;
            }
            uint32_t backrefOffset = static_cast<uint8_t>(compressedData[sourcePosition]);
            ++sourcePosition;
            if (backrefOffset == backrefByte) {
                target[targetPosition] = static_cast<char>(backrefByte);
                ++targetPosition;
            } else {
                if (backrefByte < backrefOffset) {
                    --backrefOffset;
                }
                if (backrefOffset == 0) {
                    return false;
                }
                if (sourcePosition >= compressedSize) {
                    return false;
                }
                const uint32_t backrefLength = static_cast<uint8_t>(compressedData[sourcePosition]);
                ++sourcePosition;

                if (backrefLength > 0) {
                    if (targetPosition < backrefOffset) {
                        return false;
                    }
                    if (targetPosition > UINT32_MAX - backrefLength) {
                        return false;
                    }
                    if (targetPosition + backrefLength > uncompressedSize) {
                        return false;
                    }
                    std::memmove(&target[targetPosition],
                                 &target[targetPosition - backrefOffset],
                                 backrefLength);
                    targetPosition += backrefLength;
                }
            }
        } else {
            target[targetPosition] = static_cast<char>(sourceByte);
            ++targetPosition;
        }
    }
    if (uncompressedSize != targetPosition) {
        return false;
    }

    dataBuffer = std::move(target);

    return true;
}

bool ExtractAndDecompressPkgFile(std::unique_ptr<char[]>& dataBuffer,
                                 size_t& dataLength,
                                 const PkgFile& pkgFile,
                                 HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;

    const char* compressedData = pkgFile.Data;
    uint32_t compressedLength = pkgFile.CompressedSize;
    if (!compressedData) {
        return false;
    }

    const bool hasChecksum = (pkgFile.Flags & static_cast<uint32_t>(0x2)) != 0;
    uint32_t checksum = 0;
    if (hasChecksum) {
        if (compressedLength < 4) {
            return false;
        }

        checksum = FromEndian(ReadAdvUInt32(compressedData), e);
        compressedLength -= 4;
    }

    std::unique_ptr<char[]> data;
    size_t length = 0;
    if (pkgFile.Flags & static_cast<uint32_t>(0x1)) {
        // this is the built-in compression from Falcom
        length = pkgFile.UncompressedSize;
        if (!DecompressType1(data, pkgFile.UncompressedSize, compressedData, compressedLength, e)) {
            return false;
        }
    } else if (pkgFile.Flags & static_cast<uint32_t>(0x4)) {
        // LZ4
        if (compressedLength >= 0x8000'0000 || pkgFile.UncompressedSize > LZ4_MAX_INPUT_SIZE) {
            return false;
        }
        length = pkgFile.UncompressedSize;
        data = std::make_unique_for_overwrite<char[]>(length);
        if (!data) {
            return false;
        }
        if (LZ4_decompress_safe(compressedData,
                                data.get(),
                                static_cast<int>(compressedLength),
                                static_cast<int>(pkgFile.UncompressedSize))
            != static_cast<int>(pkgFile.UncompressedSize)) {
            return false;
        }
    } else if (pkgFile.Flags & static_cast<uint32_t>(0x8)) {
        // LZMA
        if (compressedLength < 5) {
            return false;
        }

        length = pkgFile.UncompressedSize;
        data = std::make_unique_for_overwrite<char[]>(length);
        if (!data) {
            return false;
        }

        size_t outlen = length;
        size_t inlen = compressedLength - 5;
        ELzmaStatus lzmaStatus = LZMA_STATUS_NOT_SPECIFIED;
        ISzAlloc alloc{
            .Alloc = []([[maybe_unused]] ISzAllocPtr p, size_t size) { return malloc(size); },
            .Free = []([[maybe_unused]] ISzAllocPtr p, void* address) { return free(address); }};
        if (LzmaDecode((unsigned char*)data.get(),
                       &outlen,
                       (const unsigned char*)&compressedData[5],
                       &inlen,
                       (const unsigned char*)&compressedData[0],
                       5,
                       LZMA_FINISH_END,
                       &lzmaStatus,
                       &alloc)
                != SZ_OK
            || outlen != length) {
            return false;
        }
    } else if (pkgFile.Flags & static_cast<uint32_t>(0x10)) {
        // ZSTD
        length = pkgFile.UncompressedSize;
        data = std::make_unique_for_overwrite<char[]>(length);
        if (!data) {
            return false;
        }
        if (ZSTD_decompress(data.get(), length, compressedData, compressedLength) != length) {
            return false;
        }
    } else {
        // no compression
        length = compressedLength;
        data = std::make_unique_for_overwrite<char[]>(length);
        if (!data) {
            return false;
        }
        std::memcpy(data.get(), compressedData, length);
    }

    if (hasChecksum) {
        crc_t crc = static_cast<crc_t>(static_cast<uint32_t>(length));
        crc = crc_update(crc, data.get(), length);
        if (crc != checksum) {
            return false;
        }
    }

    dataBuffer = std::move(data);
    dataLength = length;

    return true;
}
} // namespace SenLib
