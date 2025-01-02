#include "pkg_compress.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "lz4/lz4.h"
#include "lz4/lz4hc.h"

#include "zstd/zstd.h"

#include "sen/pkg.h"
#include "util/hash/crc32.h"
#include "util/memwrite.h"

namespace SenLib {
static void CompressType1CalculateByteCounts(std::array<uint32_t, 256>& countPerByte,
                                             const char* uncompressedData,
                                             uint32_t uncompressedLength) {
    // The way this compression works, all bytes are just verbatim except one byte (specified in the
    // header) which is used as a command for a backref. So to determine which byte is the best
    // choice for this, we check which bytes exist in the file and then pick the one that occurs the
    // least.
    countPerByte.fill(0);
    for (size_t i = 0; i < uncompressedLength; ++i) {
        countPerByte[static_cast<uint8_t>(uncompressedData[i])] += 1;
    }
}
static uint8_t CompressType1GetLeastUsedByte(const std::array<uint32_t, 256>& countPerByte) {
    uint32_t leastUsedByte = 0;
    for (uint32_t i = 1; i < 256; ++i) {
        if (countPerByte[i] < countPerByte[leastUsedByte]) {
            leastUsedByte = i;
        }
    }
    return static_cast<uint8_t>(leastUsedByte);
}
static uint64_t CompressType1BoundForBackrefByte(const std::array<uint32_t, 256>& countPerByte,
                                                 uint32_t uncompressedLength,
                                                 uint8_t backrefByte) {
    const uint64_t bound = static_cast<uint64_t>(12) + static_cast<uint64_t>(uncompressedLength)
                           + static_cast<uint64_t>(countPerByte[backrefByte]);
    return bound;
}
// compressedData must be at least CompressType1BoundForBackrefByte() bytes long, there is no bounds
// checking for it in this function!
static bool CompressType1(char* compressedData,
                          uint32_t& compressedLength,
                          const char* uncompressedData,
                          uint32_t uncompressedLength,
                          uint8_t backrefByte) {
    size_t outputPosition = 12;

    struct Backref {
        size_t Length;
        size_t Position;
    };
    const auto find_best_backref = [&](size_t uncompressedPosition) -> Backref {
        static constexpr size_t maxBackrefLength = 255;
        static constexpr size_t maxBackrefOffset = 254;

        Backref bestBackref{0, 0};
        if (uncompressedPosition == 0) {
            return bestBackref; // no backref possible
        }

        // the game uses memcpy to evaluate the backref, so the backref *must* be completely in the
        // already written bytes. this really inhibits efficient compression of repeated byte
        // patterns, but otherwise we're risking invalid decompression by the game...

        const size_t firstPossibleBackrefPosition =
            uncompressedPosition < maxBackrefOffset ? 0 : (uncompressedPosition - maxBackrefOffset);
        const size_t lastPossibleBackrefPosition = uncompressedPosition - 1;

        size_t currentBackrefTest = lastPossibleBackrefPosition;
        const auto count_backref_from_here = [&]() -> size_t {
            size_t count = 0;
            const size_t localMaxBackrefLength = uncompressedPosition - currentBackrefTest;
            const size_t allowedBackrefLength =
                (uncompressedLength - uncompressedPosition) >= localMaxBackrefLength
                    ? localMaxBackrefLength
                    : (uncompressedLength - uncompressedPosition);
            for (size_t i = 0; i < allowedBackrefLength; ++i) {
                if (uncompressedData[currentBackrefTest + i]
                    == uncompressedData[uncompressedPosition + i]) {
                    ++count;
                } else {
                    break;
                }
            }
            return count;
        };
        while (true) {
            size_t length = count_backref_from_here();
            if (length > bestBackref.Length) {
                bestBackref.Length = length;
                bestBackref.Position = currentBackrefTest;
            }
            if (length == maxBackrefLength) {
                break;
            }

            if (currentBackrefTest == firstPossibleBackrefPosition) {
                break;
            }
            --currentBackrefTest;
        }

        return bestBackref;
    };

    const auto write_literal = [&](size_t& pos) -> void {
        const uint8_t byte = static_cast<uint8_t>(uncompressedData[pos]);
        if (byte == backrefByte) {
            compressedData[outputPosition] = static_cast<char>(byte);
            compressedData[outputPosition + 1] = static_cast<char>(byte);
            outputPosition += 2;
        } else {
            compressedData[outputPosition] = static_cast<char>(byte);
            outputPosition += 1;
        }
        pos += 1;
    };

    const auto write_backref = [&](size_t& pos, const Backref backref) -> void {
        compressedData[outputPosition] = static_cast<char>(backrefByte);
        uint8_t offset = static_cast<uint8_t>(pos - backref.Position);
        if (backrefByte <= offset) {
            ++offset;
        }
        compressedData[outputPosition + 1] = static_cast<char>(offset);
        compressedData[outputPosition + 2] =
            static_cast<char>(static_cast<uint8_t>(backref.Length));
        outputPosition += 3;
        pos += backref.Length;
    };

    size_t pos = 0;
    while (pos < uncompressedLength) {
        const Backref bestBackref = find_best_backref(pos);
        if (bestBackref.Length <= 1) {
            // could not find a (sensible) backref, need to write literal
            write_literal(pos);
            continue;
        }
        if (bestBackref.Length >= 4) {
            write_backref(pos, bestBackref);
            continue;
        }

        // we have a backref of 2 or 3 bytes; depending on what bytes are referenced, it could be
        // shorter to write the backref, or it could be shorter to write the literal bytes.
        // falcom's original compressor did not do this, so in order to get bitexact output with
        // the vanilla game files this is disabled. if you for some reason need to get a few extra
        // bytes out of this compression, feel free to enable it
        static constexpr bool checkShortBackref = false;
        if (checkShortBackref) {
            size_t literalBytesLength = 0;
            for (size_t j = 0; j < bestBackref.Length; ++j) {
                const uint8_t byte =
                    static_cast<uint8_t>(uncompressedData[bestBackref.Position + j]);
                literalBytesLength += ((byte == backrefByte) ? 2 : 1);
            }

            if (literalBytesLength <= 3) {
                write_literal(pos);
            } else {
                write_backref(pos, bestBackref);
            }
        } else {
            write_literal(pos);
        }
    }

    if (outputPosition > UINT32_MAX) {
        return false;
    }
    const uint32_t compressedLengthLocal = static_cast<uint32_t>(outputPosition);

    // from what I've seen this is always little endian, even on PS3
    using HyoutaUtils::EndianUtils::Endianness::LittleEndian;
    using HyoutaUtils::MemWrite::WriteUInt32;
    WriteUInt32(&compressedData[0],
                HyoutaUtils::EndianUtils::ToEndian(uncompressedLength, LittleEndian));
    WriteUInt32(&compressedData[4],
                HyoutaUtils::EndianUtils::ToEndian(compressedLengthLocal, LittleEndian));
    WriteUInt32(
        &compressedData[8],
        HyoutaUtils::EndianUtils::ToEndian(static_cast<uint32_t>(backrefByte), LittleEndian));

    compressedLength = compressedLengthLocal;
    return true;
}

bool CompressPkgFile(std::unique_ptr<char[]>& dataBuffer,
                     PkgFile& pkgFile,
                     const char* uncompressedData,
                     uint32_t uncompressedLength,
                     uint32_t flags,
                     HyoutaUtils::EndianUtils::Endianness e,
                     bool forceCompression) {
    const bool wantsChecksum = (flags & static_cast<uint32_t>(0x2)) != 0;

    std::unique_ptr<char[]> data;
    uint32_t length = 0;
    const uint32_t offset = wantsChecksum ? 4u : 0u;
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
        if (!forceCompression && uncompressedLength <= 12) {
            if (!write_uncompressed()) {
                return false;
            }
        } else {
            std::array<uint32_t, 256> countPerByte;
            CompressType1CalculateByteCounts(countPerByte, uncompressedData, uncompressedLength);
            const uint8_t backrefByte = CompressType1GetLeastUsedByte(countPerByte);
            const auto bound =
                CompressType1BoundForBackrefByte(countPerByte, uncompressedLength, backrefByte);
            if (bound >= SIZE_MAX - offset) {
                if (forceCompression || !write_uncompressed()) {
                    return false;
                }
            } else {
                auto compressedData =
                    std::make_unique_for_overwrite<char[]>(static_cast<size_t>(bound + offset));
                if (!compressedData) {
                    return false;
                }
                uint32_t compressedLength;
                const bool compressionSuccess = CompressType1(compressedData.get() + offset,
                                                              compressedLength,
                                                              uncompressedData,
                                                              uncompressedLength,
                                                              backrefByte);
                if (!compressionSuccess
                    || (!forceCompression && compressedLength >= uncompressedLength)) {
                    // compression failed or pointless, write uncompressed instead
                    if (forceCompression || !write_uncompressed()) {
                        return false;
                    }
                } else {
                    length = compressedLength + offset;
                    data = std::move(compressedData);
                    resultFlags = (wantsChecksum ? 2u : 0u) | 1u;
                }
            }
        }
    } else if (flags & static_cast<uint32_t>(0x4)) {
        // LZ4
        if (!forceCompression && uncompressedLength == 0) {
            if (!write_uncompressed()) {
                return false;
            }
        } else if (uncompressedLength > LZ4_MAX_INPUT_SIZE) {
            if (forceCompression || !write_uncompressed()) {
                return false;
            }
        } else {
            const int signedSize = static_cast<int>(uncompressedLength);
            const int bound = LZ4_compressBound(signedSize);
            if (bound <= 0) {
                if (forceCompression || !write_uncompressed()) {
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
                if (lz4return <= 0
                    || (!forceCompression
                        && static_cast<unsigned int>(lz4return) >= uncompressedLength)) {
                    // compression failed or pointless, write uncompressed instead
                    compressedData.reset();
                    if (forceCompression || !write_uncompressed()) {
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
        if (!forceCompression && uncompressedLength == 0) {
            if (!write_uncompressed()) {
                return false;
            }
        } else {
            size_t bound = ZSTD_compressBound(uncompressedLength);
            if (ZSTD_isError(bound)) {
                if (forceCompression || !write_uncompressed()) {
                    return false;
                }
            } else {
                auto compressedData = std::make_unique_for_overwrite<char[]>(bound + offset);
                if (!compressedData) {
                    return false;
                }
                const size_t zstdReturn = ZSTD_compress(
                    compressedData.get() + offset, bound, uncompressedData, uncompressedLength, 22);
                if (ZSTD_isError(zstdReturn)
                    || (!forceCompression && zstdReturn >= uncompressedLength)) {
                    compressedData.reset();
                    if (forceCompression || !write_uncompressed()) {
                        return false;
                    }
                } else {
                    length = zstdReturn + offset;
                    data = std::move(compressedData);
                    resultFlags = (wantsChecksum ? 2u : 0u) | 0x10u;
                }
            }
        }
    } else {
        // no compression
        if (!write_uncompressed()) {
            return false;
        }
    }

    if (wantsChecksum) {
        const uint32_t realLength = length - offset;
        crc_t crc = static_cast<crc_t>(realLength);
        crc = crc_update(crc, data.get() + offset, realLength);
        uint32_t checksum = static_cast<uint32_t>(crc);

        HyoutaUtils::MemWrite::WriteUInt32(data.get(),
                                           HyoutaUtils::EndianUtils::ToEndian(checksum, e));
    }

    pkgFile.UncompressedSize = uncompressedLength;
    pkgFile.CompressedSize = length;
    pkgFile.Flags = resultFlags;
    pkgFile.Data = data.get();
    dataBuffer = std::move(data);

    return true;
}
} // namespace SenLib
