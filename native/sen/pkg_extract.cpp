#include "pkg_extract.h"

#include <cstddef>
#include <cstring>
#include <memory>

#include "lz4/lz4.h"
#include "lzma/LzmaDec.h"
#include "zstd/zstd.h"

#include "pkg.h"
#include "util/endian.h"
#include "util/hash/crc32.h"
#include "util/memread.h"

namespace SenLib {
static bool DecompressType1(char* uncompressedDataBuffer,
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
    char* target = uncompressedDataBuffer;
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

                    // This is technically a behavior difference, the game just does a memcpy here.
                    // This makes a difference when the two ranges here overlap: What exactly memcpy
                    // does in that situation is undefined, so all the game's assets are packed in a
                    // way where that never happens. Copying one byte at a time like this works
                    // correctly even in the overlapping case.
                    for (size_t i = 0; i < backrefLength; ++i) {
                        target[targetPosition + i] = target[(targetPosition - backrefOffset) + i];
                    }
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

    return true;
}

bool ExtractAndDecompressPkgFile(char* uncompressedDataBuffer,
                                 uint32_t uncompressedDataLength,
                                 const char* compressedDataBuffer,
                                 uint32_t compressedDataLength,
                                 uint32_t flags,
                                 HyoutaUtils::EndianUtils::Endianness e) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;

    if (uncompressedDataLength == 0) {
        return true;
    }
    if (!compressedDataBuffer || !uncompressedDataBuffer) {
        return false;
    }

    const char* compressedData = compressedDataBuffer;
    uint32_t compressedLength = compressedDataLength;

    const bool hasChecksum = (flags & static_cast<uint32_t>(0x2)) != 0;
    if (hasChecksum) {
        if (compressedLength < 4) {
            return false;
        }

        uint32_t checksum = FromEndian(ReadAdvUInt32(compressedData), e);
        compressedLength -= 4;

        crc_t crc = static_cast<crc_t>(static_cast<uint32_t>(compressedLength));
        crc = crc_update(crc, compressedData, compressedLength);
        if (crc != checksum) {
            return false;
        }
    }

    size_t length = 0;
    if (flags & static_cast<uint32_t>(0x1)) {
        // this is the built-in compression from Falcom
        length = uncompressedDataLength;
        if (!DecompressType1(uncompressedDataBuffer,
                             uncompressedDataLength,
                             compressedData,
                             compressedLength,
                             e)) {
            return false;
        }
    } else if (flags & static_cast<uint32_t>(0x4)) {
        // LZ4
        if (compressedLength >= 0x8000'0000 || uncompressedDataLength > LZ4_MAX_INPUT_SIZE) {
            return false;
        }
        length = uncompressedDataLength;
        if (LZ4_decompress_safe(compressedData,
                                uncompressedDataBuffer,
                                static_cast<int>(compressedLength),
                                static_cast<int>(uncompressedDataLength))
            != static_cast<int>(uncompressedDataLength)) {
            return false;
        }
    } else if (flags & static_cast<uint32_t>(0x8)) {
        // LZMA
        if (compressedLength < 5) {
            return false;
        }

        length = uncompressedDataLength;

        size_t outlen = length;
        size_t inlen = compressedLength - 5;
        ELzmaStatus lzmaStatus = LZMA_STATUS_NOT_SPECIFIED;
        ISzAlloc alloc{
            .Alloc = []([[maybe_unused]] ISzAllocPtr p, size_t size) { return malloc(size); },
            .Free = []([[maybe_unused]] ISzAllocPtr p, void* address) { return free(address); }};
        if (LzmaDecode((unsigned char*)uncompressedDataBuffer,
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
    } else if (flags & static_cast<uint32_t>(0x10)) {
        // ZSTD
        length = uncompressedDataLength;
        if (ZSTD_decompress(uncompressedDataBuffer, length, compressedData, compressedLength)
            != length) {
            return false;
        }
    } else {
        // no compression
        length = compressedLength;
        std::memcpy(uncompressedDataBuffer, compressedData, length);
    }

    return true;
}
} // namespace SenLib
