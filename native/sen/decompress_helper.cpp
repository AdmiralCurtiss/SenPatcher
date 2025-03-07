#include "decompress_helper.h"

#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

#include "lzma/LzmaDec.h"
#include "util/align.h"
#include "util/hash/crc32.h"

namespace SenLib {
static uint16_t GetUInt16(const char* b) {
    const uint32_t b1 = static_cast<uint8_t>(b[0]);
    const uint32_t b2 = static_cast<uint8_t>(b[1]);
    return (uint16_t)(b2 << 8 | b1);
}

static uint32_t GetUInt32(const char* b) {
    const uint32_t b1 = static_cast<uint8_t>(b[0]);
    const uint32_t b2 = static_cast<uint8_t>(b[1]);
    const uint32_t b3 = static_cast<uint8_t>(b[2]);
    const uint32_t b4 = static_cast<uint8_t>(b[3]);
    return (uint32_t)(b4 << 24 | b3 << 16 | b2 << 8 | b1);
}

static void SetUInt16(char* b, uint16_t curr) {
    b[0] = (char)(curr & 0xff);
    b[1] = (char)((curr >> 8) & 0xff);
}

static void SetUInt32(char* b, uint32_t curr) {
    b[0] = (char)(curr & 0xff);
    b[1] = (char)((curr >> 8) & 0xff);
    b[2] = (char)((curr >> 16) & 0xff);
    b[3] = (char)((curr >> 24) & 0xff);
}

static uint32_t CalculateCRC32(const char* data, size_t length) {
    uint32_t crc32 = crc_init();
    crc32 = crc_update(crc32, data, length);
    crc32 = crc_finalize(crc32);
    return crc32;
}

std::optional<std::vector<char>> DecompressFromBuffer(const char* buffer, size_t bufferLength) {
    if (bufferLength <= 18) {
        return std::nullopt; // not enough bytes for header
    }

    const int filter = buffer[0];
    if (filter < 0 || filter > 3) {
        return std::nullopt; // unimplemented filter
    }

    const uint32_t crc = GetUInt32(&buffer[1]);
    const uint64_t uncompressedLength = static_cast<uint64_t>(GetUInt32(&buffer[10]))
                                        | (static_cast<uint64_t>(GetUInt32(&buffer[14])) << 32);
    std::vector<char> outbuffer;
    outbuffer.resize(uncompressedLength);

    size_t len = uncompressedLength;
    size_t compressedLength = bufferLength - 18;
    ELzmaStatus lzmaStatus = LZMA_STATUS_NOT_SPECIFIED;
    ISzAlloc alloc{
        .Alloc = []([[maybe_unused]] ISzAllocPtr p, size_t size) { return malloc(size); },
        .Free = []([[maybe_unused]] ISzAllocPtr p, void* address) { return free(address); }};
    if (LzmaDecode((unsigned char*)outbuffer.data(),
                   &len,
                   (const unsigned char*)&buffer[18],
                   &compressedLength,
                   (const unsigned char*)&buffer[5],
                   5,
                   LZMA_FINISH_END,
                   &lzmaStatus,
                   &alloc)
            != SZ_OK
        || len != uncompressedLength) {
        return std::nullopt; // decompression error
    }

    if (filter == 1) {
        if ((outbuffer.size() % 2) != 0) {
            return std::nullopt; // invalid data stream
        }

        uint16_t last = 0;
        for (size_t pos = 0x30; pos < outbuffer.size(); pos += 2) {
            const uint16_t diff = GetUInt16(&outbuffer[pos]);
            const uint16_t curr = (uint16_t)(last - diff);
            SetUInt16(&outbuffer[pos], curr);
            last = curr;
        }
    } else if (filter == 2) {
        if ((outbuffer.size() % 4) != 0) {
            return std::nullopt; // invalid data stream
        }

        uint32_t last = 0;
        for (size_t pos = 0x30; pos < outbuffer.size(); pos += 4) {
            const uint32_t diff = GetUInt32(&outbuffer[pos]);
            const uint32_t curr = (uint32_t)(last - diff);
            SetUInt32(&outbuffer[pos], curr);
            last = curr;
        }
    } else if (filter == 3) {
        if ((outbuffer.size() % 4) != 0 || outbuffer.size() < 0x30) {
            return std::nullopt; // invalid data stream
        }

        std::vector<char> defiltered;
        defiltered.resize(outbuffer.size());
        std::memcpy(defiltered.data(), outbuffer.data(), 0x30);

        size_t j = 0x30;
        uint32_t last = 0;
        for (size_t i = 0x30; i < outbuffer.size(); i += 4, j += 2) {
            const uint16_t diff = GetUInt16(&outbuffer[j]);
            const uint16_t curr = (uint16_t)(last - diff);
            SetUInt16(&defiltered[i], curr);
            last = curr;
        }
        last = 0;
        for (size_t i = 0x32; i < outbuffer.size(); i += 4, j += 2) {
            const uint16_t diff = GetUInt16(&outbuffer[j]);
            const uint16_t curr = (uint16_t)(last - diff);
            SetUInt16(&defiltered[i], curr);
            last = curr;
        }

        outbuffer.swap(defiltered);
    }

    if (CalculateCRC32(outbuffer.data(), outbuffer.size()) != crc) {
        return std::nullopt; // decompression error
    }
    return outbuffer;
}

std::optional<DecompressHelperAlignedBuffer>
    AlignedDecompressFromBuffer(const char* buffer, size_t length, size_t alignment) {
    auto decompressed = DecompressFromBuffer(buffer, length);
    if (!decompressed) {
        return std::nullopt;
    }

    DecompressHelperAlignedBuffer rv;
    rv.UnalignedData = std::move(*decompressed);
    rv.AlignedData = rv.UnalignedData.data();
    rv.Length = rv.UnalignedData.size();

    const size_t unalignedDataPos = std::bit_cast<size_t>(rv.AlignedData);
    const size_t alignedDataPos = HyoutaUtils::AlignUp(unalignedDataPos, alignment);
    if (unalignedDataPos != alignedDataPos) {
        // not aligned correctly, increase buffer and shift data
        rv.UnalignedData.resize(rv.UnalignedData.size() + alignment);

        const size_t newUnalignedDataPos = std::bit_cast<size_t>(rv.UnalignedData.data());
        const size_t newAlignedDataPos = HyoutaUtils::AlignUp(newUnalignedDataPos, alignment);
        const size_t diff = newAlignedDataPos - newUnalignedDataPos;
        rv.AlignedData = rv.UnalignedData.data() + diff;
        if (diff != 0) {
            std::memmove(rv.AlignedData, rv.UnalignedData.data(), rv.Length);
        }
    }

    return rv;
}
} // namespace SenLib
