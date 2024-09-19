#include "bra_extract.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>

#include "zlib/zlib.h"

#include "util/memread.h"

// FIXME: This would be more efficient without the intermediate 'out' buffer.
static bool InflateToBuffer(const char* inputBuffer,
                            size_t inputLength,
                            char* outputBuffer,
                            size_t outputLength) {
    // adapted from https://zlib.net/zpipe.c which is public domain
    static constexpr size_t CHUNK = 16384;

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -15);
    if (ret != Z_OK) {
        return false;
    }

    /* decompress until deflate stream ends or end of file */
    size_t bytesLeft = inputLength;
    const char* in = inputBuffer;
    char* outputBufferCurrent = outputBuffer;
    size_t outputBufferLeft = outputLength;
    do {
        strm.avail_in =
            static_cast<uInt>(std::min(bytesLeft, size_t(1) << ((sizeof(uInt) * 8) - 1)));
        if (strm.avail_in == 0)
            break;
        strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(in));

        bytesLeft -= strm.avail_in;
        in += strm.avail_in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT: ret = Z_DATA_ERROR; /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR: (void)inflateEnd(&strm); return false;
            }
            have = CHUNK - strm.avail_out;
            if (have > outputBufferLeft) {
                (void)inflateEnd(&strm);
                return false;
            }
            std::memcpy(outputBufferCurrent, out, have);
            outputBufferCurrent += have;
            outputBufferLeft -= have;
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END) {
        return false;
    }
    return outputBufferLeft == 0;
}

namespace SenPatcher {
bool ExtractAndDecompressBraFile(char* uncompressedDataBuffer,
                                 uint32_t uncompressedDataLength,
                                 const char* compressedDataBuffer,
                                 uint32_t compressedDataLength) {
    using namespace HyoutaUtils::MemRead;

    if (uncompressedDataLength == 0) {
        return true;
    }
    if (!compressedDataBuffer || !uncompressedDataBuffer) {
        return false;
    }
    if (compressedDataLength < 0x10) {
        return false;
    }

    const uint32_t uncompressedSize = ReadUInt32(&compressedDataBuffer[0x0]);
    const uint32_t compressedSize = ReadUInt32(&compressedDataBuffer[0x4]);
    const uint8_t compressionType = ReadUInt8(&compressedDataBuffer[0xc]);
    if (uncompressedSize != uncompressedDataLength) {
        return false;
    }
    if (compressedSize != (compressedDataLength - 0x10)) {
        return false;
    }

    if (compressionType == 0) {
        // uncompressed
        if ((compressedDataLength - 0x10) != uncompressedSize) {
            return false;
        }
        std::memcpy(uncompressedDataBuffer, compressedDataBuffer + 0x10, uncompressedSize);
    } else {
        // zlib compressed
        if (!InflateToBuffer(compressedDataBuffer + 0x10,
                             compressedSize,
                             uncompressedDataBuffer,
                             uncompressedDataLength)) {
            return false;
        }
    }

    return true;
}
} // namespace SenPatcher
