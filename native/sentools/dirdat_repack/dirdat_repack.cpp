#include "dirdat_repack.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "../dirdat_extract/dirdat_extract.h"
#include "dirdat_repack_main.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memwrite.h"
#include "util/scope.h"
#include "util/text.h"

namespace SenTools {
static constexpr uint16_t DefaultUncompressedChunkSize = 0xfff0;

int DirDat_Repack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output_dir{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output-dir",
        .Argument = "FILENAME",
        .Description = "The output filename for the .dir file. Must be given."};
    static constexpr HyoutaUtils::Arg arg_output_dat{
        .Type = HyoutaUtils::ArgTypes::String,
        .LongKey = "output-dat",
        .Argument = "FILENAME",
        .Description =
            "The output filename for the .dat file. Will be derived from the .dir file if not "
            "given."};
    static constexpr HyoutaUtils::Arg arg_max_chunk_size{
        .Type = HyoutaUtils::ArgTypes::UInt64,
        .LongKey = "max-chunk-size",
        .Argument = "SIZE",
        .Description =
            "The maximum uncompressed chunk size that should be used. Set to 0 for unlimited. "
            "Defaults to 0xfff0, which is the value used by the the original game files."};
    static constexpr HyoutaUtils::Arg arg_skip_bit_compression{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .LongKey = "skip-bit-compression",
        .Description = "Don't try to use the bitstream-based compression type for each chunk."};
    static constexpr HyoutaUtils::Arg arg_skip_byte_compression{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .LongKey = "skip-byte-compression",
        .Description = "Don't try to use the bytestream-based compression type for each chunk."};
    static constexpr std::array<const HyoutaUtils::Arg*, 5> args_array{
        {&arg_output_dir,
         &arg_output_dat,
         &arg_max_chunk_size,
         &arg_skip_bit_compression,
         &arg_skip_byte_compression}};
    static constexpr HyoutaUtils::Args args(
        "sentools " DirDat_Repack_Name,
        "__dirdat.json",
        DirDat_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted archive and keeps all the metadata as best as "
        "possible. For example, the file order and all the unidentified header fields will be "
        "preserved.\n\n"
        "To use this, extract with the -j option, then point this program at the __dirdat.json "
        "that was generated during the archive extraction.",
        args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input file given."
                                                 : "More than 1 input file given.");
        args.PrintUsage();
        return -1;
    }

    auto* output_option_dir = options.TryGetString(&arg_output_dir);
    if (output_option_dir == nullptr) {
        printf("Argument error: %s\n\n\n", "No output filename given.");
        args.PrintUsage();
        return -1;
    }

    const bool skipBitCompression = options.IsFlagSet(&arg_skip_bit_compression);
    const bool skipByteCompression = options.IsFlagSet(&arg_skip_byte_compression);

    auto* max_chunk_size_option = options.TryGetUInt64(&arg_max_chunk_size);
    size_t maxChunkSize = DefaultUncompressedChunkSize;
    if (max_chunk_size_option != nullptr) {
        if (*max_chunk_size_option <= std::numeric_limits<size_t>::max()) {
            maxChunkSize = static_cast<size_t>(*max_chunk_size_option);
        }
    }

    std::string_view sourcePath(options.FreeArguments[0]);
    std::string_view targetPathDir(*output_option_dir);
    std::string_view targetPathDat;
    std::string targetPathDatStorage;
    if (auto* output_option_dat = options.TryGetString(&arg_output_dat)) {
        targetPathDat = std::string_view(*output_option_dat);
    } else {
        const size_t sep = targetPathDir.find_last_of(
#ifdef BUILD_FOR_WINDOWS
            "\\"
#endif
            "/.");
        if (sep != std::string_view::npos && targetPathDir[sep] == '.') {
            targetPathDatStorage = std::string(targetPathDir.substr(0, sep)) + ".dat";
        } else {
            targetPathDatStorage = std::string(targetPathDir) + ".dat";
        }
        targetPathDat = targetPathDatStorage;
    }

    auto result = DirDat::RepackDirDat(sourcePath,
                                       targetPathDir,
                                       targetPathDat,
                                       maxChunkSize,
                                       skipBitCompression,
                                       skipByteCompression);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}
} // namespace SenTools

namespace SenTools::DirDat {
template<typename T>
static std::optional<uint32_t> JsonReadUInt32(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsUint()) {
        const auto i = j.GetUint();
        return static_cast<uint32_t>(i);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<std::string> JsonReadString(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsString()) {
        const char* str = j.GetString();
        const auto len = j.GetStringLength();
        return std::string(str, len);
    }
    return std::nullopt;
}

template<typename T>
static std::optional<bool> JsonReadBool(T& json, const char* key) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) {
        return std::nullopt;
    }
    auto& j = it->value;
    if (j.IsBool()) {
        return j.GetBool();
    }
    return std::nullopt;
}

struct SingleFileDirForPacking {
    SingleFileDir Metadata;
    std::optional<std::filesystem::path> Path;
    bool ShouldCompress = false;
    bool ShouldOverrideCompressedSizeCopy = false;
};

// for our bound, we need to consider the case where the data is uncompressable. unfortunately
// the format does not give us a way to say 'the file is uncompressed' so we must compress it
// even if that increases the size. the best way to do this is, as far as I can tell:
// - use the byte-based compression (DecompressChunk1)
// - repeatedly encode 8191 bytes of direct copy from the compressed stream
// annoyingly, with the default uncompressed chunk size of 0xfff0 this results in a compressed
// chunk size of 0x10003 bytes, which *does not fit*, so if we encounter this we must reduce the
// input chunk length by 3 bytes. so this results in a bound of...
static constexpr size_t MaxChunkSizeForUncompressable = 0xfff0 - 3;
size_t CompressedFileBound(size_t uncompressedLength, size_t maxChunkSize) {
    if (uncompressedLength == 0) {
        return 6;
    }
    if (maxChunkSize == 0 || maxChunkSize >= MaxChunkSizeForUncompressable) {
        const size_t worstChunkCount = ((uncompressedLength + (MaxChunkSizeForUncompressable - 1))
                                        / MaxChunkSizeForUncompressable);
        const size_t bound = worstChunkCount * 0x10000;
        return bound;
    } else {
        static constexpr size_t maxLiteralLength = 8191;
        const size_t requiredFullLiterals = maxChunkSize / maxLiteralLength;
        const size_t lengthOfRemainingLiteral = maxChunkSize % maxLiteralLength;
        const size_t overheadPerFullLiteral = (maxChunkSize <= 31u ? 1u : 2u);
        const size_t uncompressedChunkOverhead =
            (requiredFullLiterals * overheadPerFullLiteral)
            + (lengthOfRemainingLiteral == 0u ? 0u : (lengthOfRemainingLiteral <= 31u ? 1u : 2u));
        const size_t lengthOfCompressedChunk = (maxChunkSize + uncompressedChunkOverhead + 3u);
        const size_t worstChunkCount = ((uncompressedLength + (maxChunkSize - 1)) / maxChunkSize);
        const size_t bound = worstChunkCount * lengthOfCompressedChunk;
        return bound;
    }
}

#define CHECK_RANGE_COMPRESSED(n)                                                              \
    do {                                                                                       \
        if ((n) > compressedBufferLength || compressedOffset > compressedBufferLength - (n)) { \
            return std::nullopt;                                                               \
        }                                                                                      \
    } while (false)

// returns std::nullopt on failure. otherwise returns length of compressed data.
std::optional<uint32_t> CompressChunk0(const char* uncompressed,
                                       size_t uncompressedLength,
                                       char* compressed,
                                       size_t compressedBufferLength,
                                       size_t* outUncompressedBytesUsed) {
    // note: this is heavily based on the compression function I wrote for Tales of Phantasia,
    // the formats are pretty similar. see https://github.com/AdmiralCurtiss/topdec

    static constexpr size_t minBackrefLength = 2;
    static constexpr size_t maxBackrefLength = 269;
    static constexpr size_t minBackrefOffset = 1;
    static constexpr size_t maxBackrefOffset = 8191;
    static constexpr size_t minSameByteLength = 14;
    static constexpr size_t maxSameByteLength = 4109;


    size_t compressedOffset = 0;
    size_t uncompressedOffset = 0;

    int bitsWritten = 0;
    size_t commandBitPosition = 0;

#define WRITE_BIT(bit)                                                                           \
    do {                                                                                         \
        if (bitsWritten == 0) {                                                                  \
            CHECK_RANGE_COMPRESSED(2);                                                           \
            compressed[compressedOffset] = 0;                                                    \
            commandBitPosition = compressedOffset;                                               \
            ++compressedOffset;                                                                  \
            compressed[compressedOffset] = 0;                                                    \
            ++compressedOffset;                                                                  \
        }                                                                                        \
                                                                                                 \
        assert((bit) == 0 || (bit) == 1);                                                        \
        if (bitsWritten < 8) {                                                                   \
            compressed[commandBitPosition] |= static_cast<char>((bit) << bitsWritten);           \
        } else {                                                                                 \
            compressed[commandBitPosition + 1] |= static_cast<char>((bit) << (bitsWritten - 8)); \
        }                                                                                        \
                                                                                                 \
        bitsWritten = (bitsWritten + 1) & 15;                                                    \
    } while (false)

#define WRITE_N_BITS(bits, count)                                \
    do {                                                         \
        const auto ppbits = (bits);                              \
        for (int i = (count) - 1; i >= 0; --i) {                 \
            const int bit = static_cast<int>((ppbits >> i) & 1); \
            WRITE_BIT(bit);                                      \
        }                                                        \
    } while (false)

#define WRITE_LITERAL()                                                  \
    do {                                                                 \
        WRITE_BIT(0);                                                    \
        CHECK_RANGE_COMPRESSED(1);                                       \
        compressed[compressedOffset] = uncompressed[uncompressedOffset]; \
        ++compressedOffset;                                              \
        ++uncompressedOffset;                                            \
    } while (false)

    // same byte is triggered by writing a long backref offset of exactly 1
#define WRITE_SAME_BYTE(count)                                                     \
    do {                                                                           \
        WRITE_BIT(1);                                                              \
        WRITE_BIT(1);                                                              \
        WRITE_N_BITS(0, 5);                                                        \
        CHECK_RANGE_COMPRESSED(1);                                                 \
        compressed[compressedOffset] = 1;                                          \
        ++compressedOffset;                                                        \
                                                                                   \
        assert((count) >= minSameByteLength && (count) <= maxSameByteLength);      \
        const size_t countToWrite = (count) - minSameByteLength;                   \
                                                                                   \
        if (countToWrite <= 0xf) {                                                 \
            WRITE_BIT(0);                                                          \
            WRITE_N_BITS(countToWrite, 4);                                         \
        } else {                                                                   \
            WRITE_BIT(1);                                                          \
            WRITE_N_BITS(countToWrite >> 8, 4);                                    \
            CHECK_RANGE_COMPRESSED(1);                                             \
            compressed[compressedOffset] = static_cast<char>(countToWrite & 0xff); \
            ++compressedOffset;                                                    \
        }                                                                          \
        CHECK_RANGE_COMPRESSED(1);                                                 \
        compressed[compressedOffset] = uncompressed[uncompressedOffset];           \
        ++compressedOffset;                                                        \
        uncompressedOffset += (count);                                             \
    } while (false)

    const auto count_same_byte = [&]() -> size_t {
        size_t count = 0;
        const char c = uncompressed[uncompressedOffset];
        for (size_t i = uncompressedOffset; i < uncompressedLength; ++i) {
            if (uncompressed[i] == c) {
                ++count;
            } else {
                break;
            }
        }
        return count > maxSameByteLength ? maxSameByteLength : count;
    };

    struct Backref {
        size_t Length;
        size_t Position;
    };
    struct EncodedLength {
        size_t NumBits;
        size_t NumBytes;
    };

#define WRITE_BACKREF(length, offset)                                                 \
    do {                                                                              \
        WRITE_BIT(1);                                                                 \
                                                                                      \
        assert((length) >= minBackrefLength && (length) <= maxBackrefLength);         \
        assert((offset) >= minBackrefOffset && (offset) <= maxBackrefOffset);         \
                                                                                      \
        if ((offset) <= 0xff) {                                                       \
            WRITE_BIT(0);                                                             \
        } else {                                                                      \
            WRITE_BIT(1);                                                             \
            WRITE_N_BITS((offset) >> 8, 5);                                           \
        }                                                                             \
        CHECK_RANGE_COMPRESSED(1);                                                    \
        compressed[compressedOffset] = static_cast<char>((offset) & 0xff);            \
        ++compressedOffset;                                                           \
                                                                                      \
        if ((length) == 2) {                                                          \
            WRITE_BIT(1);                                                             \
        } else if ((length) == 3) {                                                   \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(1);                                                             \
        } else if ((length) == 4) {                                                   \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(1);                                                             \
        } else if ((length) == 5) {                                                   \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(1);                                                             \
        } else if ((length) <= 13) {                                                  \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(1);                                                             \
            WRITE_N_BITS((length) - 6, 3);                                            \
        } else {                                                                      \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            WRITE_BIT(0);                                                             \
            CHECK_RANGE_COMPRESSED(1);                                                \
            compressed[compressedOffset] = static_cast<char>(((length) - 14) & 0xff); \
            ++compressedOffset;                                                       \
        }                                                                             \
                                                                                      \
        uncompressedOffset += (length);                                               \
    } while (false)

    const auto find_best_backref = [&]() -> Backref {
        Backref bestBackref{0, 0};

        if (uncompressedOffset < minBackrefOffset) {
            return bestBackref; // no backref possible
        }

        const size_t firstPossibleBackrefPosition =
            uncompressedOffset < maxBackrefOffset ? 0 : (uncompressedOffset - maxBackrefOffset);
        const size_t lastPossibleBackrefPosition = uncompressedOffset - 1;
        const size_t allowedBackrefLength =
            (uncompressedLength - uncompressedOffset) >= maxBackrefLength
                ? maxBackrefLength
                : (uncompressedLength - uncompressedOffset);

        if (allowedBackrefLength < minBackrefLength) {
            return bestBackref; // no backref possible
        }

        size_t currentBackrefTest = lastPossibleBackrefPosition;
        const auto count_backref_from_here = [&]() -> size_t {
            size_t count = 0;
            for (size_t i = 0; i < allowedBackrefLength; ++i) {
                if (uncompressed[currentBackrefTest + i] == uncompressed[uncompressedOffset + i]) {
                    ++count;
                } else {
                    break;
                }
            }
            return count;
        };
        while (true) {
            const size_t length = count_backref_from_here();
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

    // initialize the bit counter logic and writes the type byte
    WRITE_N_BITS(0, 8);

    // we have three options for writing data, pick the one that takes the least bytes.
    // this is mostly greedy. maybe a non-greedy algorithm could save a few bytes sometimes...?

    // writing a same byte takes:
    // - 7 bits + 1 byte to enter the mode
    // - depending on the length,
    //   - 5 bits for a short length (<= 29)
    //   - 5 bits and 1 byte for a long length
    // - 1 byte for the actual byte
    const auto calc_same_byte_encoded_length = [](size_t len) -> EncodedLength {
        return EncodedLength{.NumBits = 7u + 5u, .NumBytes = 1u + (len <= 29 ? 0u : 1u) + 1u};
    };

    // writing a backref takes:
    // - 1 bit to enter the mode
    // - depending on the offset,
    //   - 1 bit and 1 byte for a short offset (<= 255)
    //   - 6 bits and 1 byte for a long offset
    // - depending on the length,
    //   - 1 bit for length == 2
    //   - 2 bits for length == 3
    //   - 3 bits for length == 4
    //   - 4 bits for length == 5
    //   - 8 bits for length <= 13
    //   - 5 bits and 1 byte for anything above that
    const auto calc_backref_encoded_length = [](size_t len, size_t offset) -> EncodedLength {
        const size_t bitsForOffset = (offset <= 255u ? 1u : 6u);
        static constexpr size_t bytesForOffset = 1u;
        const size_t bitsForLength = (len == 2)    ? 1u
                                     : (len == 3)  ? 2u
                                     : (len == 4)  ? 3u
                                     : (len == 5)  ? 4u
                                     : (len <= 13) ? 8u
                                                   : 5u;
        const size_t bytesForLength = (len <= 13) ? 0u : 1u;
        return EncodedLength{.NumBits = 1u + bitsForOffset + bitsForLength,
                             .NumBytes = bytesForOffset + bytesForLength};
    };

    // writing a literal takes 9 bits

    const auto fits_in_remaining_compressed_buffer = [&](const EncodedLength& len) -> bool {
        // we must include the 'end of compressed data' marker, which takes 7 bits and 1 byte.
        // first convert the bits into actual bytes written. this is somewhat complicated since
        // bits are written in groups of 16 and we may be in the middle of a group, so...
        const size_t bits = 7u + len.NumBits;
        const size_t freeBits = (16u - bitsWritten) & 15u;
        const size_t remainingBits = bits <= freeBits ? 0u : (bits - freeBits);
        const size_t convertedBits = (remainingBits + 15u) / 16u;
        // the rest is straightforward.
        const size_t bytes = 1u + len.NumBytes + convertedBits;
        return !(bytes > compressedBufferLength
                 || compressedOffset > (compressedBufferLength - bytes));
    };

    while (uncompressedOffset < uncompressedLength) {
        const size_t sameByteCount = count_same_byte();
        const Backref bestBackref = find_best_backref();
        const bool sameByteCountValid = sameByteCount >= minSameByteLength;
        const bool backrefValid = bestBackref.Length >= minBackrefLength;

        // if you math this out you realize that no matter the offset and lengths, writing a backref
        // is always better than or equivalent to the same byte sequence. writing a backref is also
        // always better than writing literals, even for very short backrefs with long offsets.

        // there's one fairly non-intuitive case: a backref of (n-1) bytes can sometimes beat a same
        // byte sequence of (n) bytes, if the backref has a short offset but the same byte has the
        // long length: the backref takes 23 bits to encode in that case, but the same byte takes
        // 36, which actually saves 4 bits to write the backref followed by a literal

        const bool wantBackref =
            backrefValid
            && (!sameByteCountValid || bestBackref.Length >= sameByteCount
                || (bestBackref.Length == (sameByteCount - 1) && sameByteCount > 29u
                    && (uncompressedOffset - bestBackref.Position) <= 255u));
        if (wantBackref) {
            const size_t offset = uncompressedOffset - bestBackref.Position;
            const EncodedLength encodedLength =
                calc_backref_encoded_length(bestBackref.Length, offset);
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            WRITE_BACKREF(bestBackref.Length, offset);
        } else if (sameByteCountValid) {
            const EncodedLength encodedLength = calc_same_byte_encoded_length(sameByteCount);
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            WRITE_SAME_BYTE(sameByteCount);
        } else {
            const EncodedLength encodedLength{.NumBits = 1u, .NumBytes = 1u};
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            WRITE_LITERAL();
        }
    }

    // to finish we must write the end-of-file sequence
    WRITE_BIT(1);
    WRITE_BIT(1);
    WRITE_N_BITS(0, 5);
    CHECK_RANGE_COMPRESSED(1);
    compressed[compressedOffset] = 0;
    ++compressedOffset;

    *outUncompressedBytesUsed = uncompressedOffset;
    return compressedOffset;

#undef WRITE_BACKREF
#undef WRITE_SAME_BYTE
#undef WRITE_LITERAL
#undef WRITE_N_BITS
#undef WRITE_BIT
}

// returns std::nullopt on failure. otherwise returns length of compressed data.
std::optional<uint32_t> CompressChunk1(const char* uncompressed,
                                       size_t uncompressedLength,
                                       char* compressed,
                                       size_t compressedBufferLength,
                                       size_t* outUncompressedBytesUsed) {
    static constexpr size_t minBackrefLength = 4;
    static constexpr size_t maxBackrefLength = std::numeric_limits<size_t>::max(); // unbounded
    static constexpr size_t minBackrefOffset = 1;
    static constexpr size_t maxBackrefOffset = 8191;
    static constexpr size_t minSameByteLength = 4;
    static constexpr size_t maxSameByteLength = 4099;
    static constexpr size_t maxLiteralLength = 8191;


    size_t compressedOffset = 0;
    size_t uncompressedOffset = 0;
    size_t stashedLiteralBytes = 0;

#define WRITE_LITERALS(length, position)                                                       \
    do {                                                                                       \
        assert((length) >= 1 && (length) <= maxLiteralLength);                                 \
        if ((length) <= 31) {                                                                  \
            CHECK_RANGE_COMPRESSED(1);                                                         \
            compressed[compressedOffset] = static_cast<char>(length);                          \
            ++compressedOffset;                                                                \
        } else {                                                                               \
            CHECK_RANGE_COMPRESSED(2);                                                         \
            compressed[compressedOffset] = static_cast<char>(0x20 | (((length) >> 8) & 0x1f)); \
            ++compressedOffset;                                                                \
            compressed[compressedOffset] = static_cast<char>((length) & 0xff);                 \
            ++compressedOffset;                                                                \
        }                                                                                      \
        CHECK_RANGE_COMPRESSED(length);                                                        \
        size_t rest = (length);                                                                \
        size_t pos = (position);                                                               \
        while (rest > 0) {                                                                     \
            compressed[compressedOffset] = uncompressed[pos];                                  \
            ++compressedOffset;                                                                \
            ++pos;                                                                             \
            --rest;                                                                            \
        }                                                                                      \
    } while (false)

#define FLUSH_LITERALS()                                                                           \
    do {                                                                                           \
        size_t literalOffset = uncompressedOffset - stashedLiteralBytes;                           \
        while (stashedLiteralBytes > 0) {                                                          \
            const size_t count =                                                                   \
                (stashedLiteralBytes > maxLiteralLength) ? maxLiteralLength : stashedLiteralBytes; \
            WRITE_LITERALS(count, literalOffset);                                                  \
            literalOffset += count;                                                                \
            stashedLiteralBytes -= count;                                                          \
        }                                                                                          \
    } while (false)

    // same byte is triggered by writing a long backref offset of exactly 1
#define WRITE_SAME_BYTE(count)                                                       \
    do {                                                                             \
        assert((count) >= minSameByteLength && (count) <= maxSameByteLength);        \
        const size_t countToWrite = (count) - minSameByteLength;                     \
                                                                                     \
        if (countToWrite <= 0xf) {                                                   \
            CHECK_RANGE_COMPRESSED(1);                                               \
            compressed[compressedOffset] = static_cast<char>(0x40 | countToWrite);   \
            ++compressedOffset;                                                      \
        } else {                                                                     \
            CHECK_RANGE_COMPRESSED(2);                                               \
            compressed[compressedOffset] =                                           \
                static_cast<char>(0x50 | (((countToWrite) >> 8) & 0xf));             \
            ++compressedOffset;                                                      \
            compressed[compressedOffset] = static_cast<char>((countToWrite) & 0xff); \
            ++compressedOffset;                                                      \
        }                                                                            \
                                                                                     \
        CHECK_RANGE_COMPRESSED(1);                                                   \
        compressed[compressedOffset] = uncompressed[uncompressedOffset];             \
        ++compressedOffset;                                                          \
        uncompressedOffset += (count);                                               \
    } while (false)

    const auto count_same_byte = [&]() -> size_t {
        size_t count = 0;
        const char c = uncompressed[uncompressedOffset];
        for (size_t i = uncompressedOffset; i < uncompressedLength; ++i) {
            if (uncompressed[i] == c) {
                ++count;
            } else {
                break;
            }
        }
        return count > maxSameByteLength ? maxSameByteLength : count;
    };

    struct Backref {
        size_t Length;
        size_t Position;
    };

#define WRITE_BACKREF(length, offset)                                                \
    do {                                                                             \
        assert((length) >= minBackrefLength && (length) <= maxBackrefLength);        \
        assert((offset) >= minBackrefOffset && (offset) <= maxBackrefOffset);        \
                                                                                     \
        size_t restLength = (length) - 4;                                            \
        const size_t startLength = restLength > 3 ? 3 : restLength;                  \
                                                                                     \
        CHECK_RANGE_COMPRESSED(2);                                                   \
        compressed[compressedOffset] =                                               \
            static_cast<char>(0x80 | (startLength << 5) | (((offset) >> 8) & 0x1f)); \
        ++compressedOffset;                                                          \
        compressed[compressedOffset] = static_cast<char>((offset) & 0xff);           \
        ++compressedOffset;                                                          \
                                                                                     \
        restLength -= startLength;                                                   \
        while (restLength > 0) {                                                     \
            if (restLength >= 0x1f) {                                                \
                CHECK_RANGE_COMPRESSED(1);                                           \
                compressed[compressedOffset] = static_cast<char>(0x60 | 0x1f);       \
                ++compressedOffset;                                                  \
                restLength -= 0x1f;                                                  \
            } else {                                                                 \
                CHECK_RANGE_COMPRESSED(1);                                           \
                compressed[compressedOffset] = static_cast<char>(0x60 | restLength); \
                ++compressedOffset;                                                  \
                break;                                                               \
            }                                                                        \
        }                                                                            \
                                                                                     \
        uncompressedOffset += (length);                                              \
    } while (false)

    const auto find_best_backref = [&]() -> Backref {
        Backref bestBackref{0, 0};

        if (uncompressedOffset < minBackrefOffset) {
            return bestBackref; // no backref possible
        }

        const size_t firstPossibleBackrefPosition =
            uncompressedOffset < maxBackrefOffset ? 0 : (uncompressedOffset - maxBackrefOffset);
        const size_t lastPossibleBackrefPosition = uncompressedOffset - 1;
        const size_t allowedBackrefLength =
            (uncompressedLength - uncompressedOffset) >= maxBackrefLength
                ? maxBackrefLength
                : (uncompressedLength - uncompressedOffset);

        if (allowedBackrefLength < minBackrefLength) {
            return bestBackref; // no backref possible
        }

        size_t currentBackrefTest = lastPossibleBackrefPosition;
        const auto count_backref_from_here = [&]() -> size_t {
            size_t count = 0;
            for (size_t i = 0; i < allowedBackrefLength; ++i) {
                if (uncompressed[currentBackrefTest + i] == uncompressed[uncompressedOffset + i]) {
                    ++count;
                } else {
                    break;
                }
            }
            return count;
        };
        while (true) {
            const size_t length = count_backref_from_here();
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

    // same thing here. this compression can fairly cheaply encode long sequences of literals.

    // writing a same byte takes:
    // - depending on the length,
    //   - 1 byte for a short length (<= 19)
    //   - 2 bytes for a long length
    // - 1 byte for the byte itself
    const auto calc_same_byte_encoded_length = [](size_t len) -> size_t {
        return (len <= 19u) ? 2u : 3u;
    };

    // writing a backref takes:
    // - 2 bytes for a short length (<= 7)
    // - ceil((length - 7) / 31) extra bytes for anything longer than that
    const auto calc_backref_encoded_length = [](size_t len) -> size_t {
        if (len <= 7u) {
            return 2u;
        }
        return (((len - 7u) + 30u) / 31u);
    };

    // writing literals takes:
    // - depending on the amount of literals,
    //   - (n + 1) bytes for <= 31 literals
    //   - (n + 2) bytes above that
    const auto calc_literals_encoded_length = [](size_t len) -> size_t {
        return (len <= 31u) ? (len + 1u) : (len + 2u);
    };
    const auto calc_literals_encoded_length_multiblock = [](size_t len) -> size_t {
        const size_t requiredFullLiterals = len / maxLiteralLength;
        const size_t lengthOfRemainingLiteral = len % maxLiteralLength;
        const size_t overhead =
            (requiredFullLiterals * 2u)
            + (lengthOfRemainingLiteral == 0u ? 0u : (lengthOfRemainingLiteral <= 31u ? 1u : 2u));
        return (len + overhead);
    };

    const auto fits_in_remaining_compressed_buffer = [&](size_t len) -> bool {
        return !(len > compressedBufferLength || compressedOffset > (compressedBufferLength - len));
    };

    while (uncompressedOffset < uncompressedLength) {
        const size_t sameByteCount = count_same_byte();
        const Backref bestBackref = find_best_backref();
        const bool sameByteCountValid = sameByteCount >= minSameByteLength;
        const bool backrefValid = bestBackref.Length >= minBackrefLength;

        // unlike the other algorithm, this one prefers long same byte sequences over backrefs.
        // the minimum lengths are picked so that it's always better to interrupt a literal sequence
        // to write a backref or same byte when possible.

        if (stashedLiteralBytes > 0 && (sameByteCountValid || backrefValid)) {
            FLUSH_LITERALS();
        }

        const bool wantSameByte = [&]() -> bool {
            if (!sameByteCountValid) {
                return false;
            }
            if (!backrefValid) {
                return true;
            }
            if (sameByteCount >= bestBackref.Length) {
                return true;
            }

            // it may still be preferable to use the same byte over the backref, just because the
            // backref has this terrible successive length penality...
            const size_t restLength = (bestBackref.Length - sameByteCount);
            const size_t bestBackrefEncodedLength = calc_backref_encoded_length(bestBackref.Length);
            const size_t sameByteEncodedLength = calc_same_byte_encoded_length(sameByteCount);
            const size_t restEncodedLength = restLength < minBackrefLength
                                                 ? calc_literals_encoded_length(restLength)
                                                 : calc_backref_encoded_length(restLength);
            if (sameByteEncodedLength + restEncodedLength <= bestBackrefEncodedLength) {
                return true;
            }
            return false;
        }();

        if (wantSameByte) {
            const size_t encodedLength = calc_same_byte_encoded_length(sameByteCount);
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            WRITE_SAME_BYTE(sameByteCount);
        } else if (backrefValid) {
            const size_t encodedLength = calc_backref_encoded_length(bestBackref.Length);
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            const size_t offset = uncompressedOffset - bestBackref.Position;
            WRITE_BACKREF(bestBackref.Length, offset);
        } else {
            const size_t encodedLength =
                calc_literals_encoded_length_multiblock(stashedLiteralBytes + 1u);
            if (!fits_in_remaining_compressed_buffer(encodedLength)) {
                break;
            }
            ++stashedLiteralBytes;
            ++uncompressedOffset;
        }
    }
    if (stashedLiteralBytes > 0) {
        FLUSH_LITERALS();
    }

    *outUncompressedBytesUsed = uncompressedOffset;
    return compressedOffset;

#undef WRITE_BACKREF
#undef WRITE_SAME_BYTE
#undef FLUSH_LITERALS
#undef WRITE_LITERALS
}

// returns std::nullopt on failure. otherwise returns length of compressed data.
std::optional<uint32_t> WriteUncompressableChunk(const char* uncompressedData,
                                                 size_t uncompressedChunkLength,
                                                 char* compressedData,
                                                 size_t compressedBufferLength) {
    size_t uncompressedChunkRest = uncompressedChunkLength;
    size_t compressedOffset = 0;
    size_t uncompressedOffset = 0;
    while (uncompressedChunkRest > 0) {
        uint16_t len = std::min<size_t>(0x1fff, uncompressedChunkRest);
        if (len <= 31) {
            CHECK_RANGE_COMPRESSED(1);
            compressedData[compressedOffset] = (len & 0xff);
            ++compressedOffset;
        } else {
            CHECK_RANGE_COMPRESSED(2);
            compressedData[compressedOffset] = (len >> 8) | 0x20;
            ++compressedOffset;
            compressedData[compressedOffset] = (len & 0xff);
            ++compressedOffset;
        }
        CHECK_RANGE_COMPRESSED(len);
        for (int i = 0; i < len; ++i) {
            compressedData[compressedOffset] = uncompressedData[uncompressedOffset];
            ++compressedOffset;
            ++uncompressedOffset;
        }
        uncompressedChunkRest -= len;
    }
    assert(uncompressedOffset == uncompressedChunkLength);
    return compressedOffset;
}

// returns std::nullopt if compression failed. otherwise returns length of compressed data.
std::optional<uint32_t> CompressFile(const char* uncompressedData,
                                     size_t uncompressedLength,
                                     char* compressedData,
                                     size_t compressedBufferLength,
                                     size_t maxChunkSize,
                                     bool skipBitCompression,
                                     bool skipByteCompression,
                                     HyoutaUtils::EndianUtils::Endianness endian) {
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::ToEndian;
    if (uncompressedLength == 0) {
        // the best I can come up with for this case that decompresses correctly is:
        if (compressedBufferLength < 6) {
            return std::nullopt;
        }
        const uint16_t emptyChunkLength = 5;
        WriteUInt16(&compressedData[0], ToEndian(emptyChunkLength, endian));
        compressedData[2] = 0; // bit-based compression (DecompressChunk0)
        compressedData[3] = 3; // two 1s and five 0s in the bits
        compressedData[4] = 0; // zero length for the offsetBehind byte
        compressedData[5] = 0; // no further chunks
        return 6;
    }

    // the vanilla game always uses uncompressed chunks of 0xfff0 bytes each, except for the last
    // one which is whatever is left. as far as I can tell the size doesn't actually matter as long
    // as the compressed data length fits in the two bytes for the chunk length, but might as well
    // stick with this when possible.

    size_t uncompressedRest = uncompressedLength;
    size_t compressedOffset = 0;
    size_t uncompressedOffset = 0;
    while (uncompressedRest > 0) {
        size_t uncompressedChunkLength =
            maxChunkSize == 0 ? uncompressedRest : std::min<size_t>(maxChunkSize, uncompressedRest);
        uint16_t compressedChunkLength; // this is without the trailing 'more chunks?' marker
        // reserve 2 bytes for the chunk length, 1 byte for the marker
        size_t compressedRest = compressedBufferLength - compressedOffset;
        if (compressedRest < 3) {
            // again, minimum length of compressed chunk
            return std::nullopt;
        }
        size_t spaceForCompressedData = compressedRest - 3;

        // try both algorithms and pick the better compression
        std::array<char, 0xffff - 2> tmpBuffer0;
        std::array<char, 0xffff - 2> tmpBuffer1;
        size_t outUncompressedBytesUsed0 = 0;
        std::optional<uint32_t> compressedChunkSize0 =
            skipBitCompression ? std::nullopt
                               : CompressChunk0(uncompressedData + uncompressedOffset,
                                                uncompressedChunkLength,
                                                tmpBuffer0.data(),
                                                std::min(tmpBuffer0.size(), spaceForCompressedData),
                                                &outUncompressedBytesUsed0);
        size_t outUncompressedBytesUsed1 = 0;
        std::optional<uint32_t> compressedChunkSize1 =
            skipByteCompression
                ? std::nullopt
                : CompressChunk1(uncompressedData + uncompressedOffset,
                                 uncompressedChunkLength,
                                 tmpBuffer1.data(),
                                 std::min(tmpBuffer1.size(), spaceForCompressedData),
                                 &outUncompressedBytesUsed1);
        const bool wantChunk1 = [&]() -> bool {
            if (!compressedChunkSize1.has_value()) {
                return false;
            }
            if (!compressedChunkSize0.has_value()) {
                return true;
            }

            // typical case where both algorithms take the entire input chunk
            if (outUncompressedBytesUsed0 == outUncompressedBytesUsed1) {
                return (*compressedChunkSize1 <= *compressedChunkSize0);
            }

            // otherwise we're in a situation where we got limited by the maximum compressed
            // chunk size. take the chunk that consumed the most uncompressed bytes.
            return (outUncompressedBytesUsed1 >= outUncompressedBytesUsed0);
        }();
        size_t uncompressedBytesUsed = 0;
        if (wantChunk1) {
            assert(*compressedChunkSize1 <= (0xffff - 2));
            compressedChunkLength = *compressedChunkSize1 + 2;
            std::memcpy(
                compressedData + compressedOffset + 2, tmpBuffer1.data(), *compressedChunkSize1);
            uncompressedBytesUsed = outUncompressedBytesUsed1;
        } else if (compressedChunkSize0.has_value()) {
            assert(*compressedChunkSize0 <= (0xffff - 2));
            compressedChunkLength = *compressedChunkSize0 + 2;
            std::memcpy(
                compressedData + compressedOffset + 2, tmpBuffer0.data(), *compressedChunkSize0);
            uncompressedBytesUsed = outUncompressedBytesUsed0;
        } else {
            uncompressedChunkLength = std::min<size_t>(
                maxChunkSize == 0 ? MaxChunkSizeForUncompressable
                                  : std::min<size_t>(maxChunkSize, MaxChunkSizeForUncompressable),
                uncompressedRest);
            std::optional<uint32_t> compressedChunkSize =
                WriteUncompressableChunk(uncompressedData + uncompressedOffset,
                                         uncompressedChunkLength,
                                         compressedData + (compressedOffset + 2),
                                         spaceForCompressedData);
            if (!compressedChunkSize.has_value() || *compressedChunkSize > (0xffff - 2)) {
                return std::nullopt;
            }
            compressedChunkLength = *compressedChunkSize + 2;
            uncompressedBytesUsed = uncompressedChunkLength;
        }

        WriteUInt16(&compressedData[compressedOffset], ToEndian(compressedChunkLength, endian));
        compressedOffset += compressedChunkLength;
        uncompressedOffset += uncompressedBytesUsed;
        uncompressedRest -= uncompressedBytesUsed;
        bool moreChunks = (uncompressedRest > 0);
        compressedData[compressedOffset] =
            (moreChunks ? static_cast<char>(1) : static_cast<char>(0));
        ++compressedOffset;
    }

    return compressedOffset;
}

HyoutaUtils::Result<RepackDirDatResult, std::string> RepackDirDat(std::string_view sourcePath,
                                                                  std::string_view targetPathDir,
                                                                  std::string_view targetPathDat,
                                                                  size_t maxChunkSize,
                                                                  bool skipBitCompression,
                                                                  bool skipByteCompression) {
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::ToEndian;
    static constexpr auto endian = HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    std::filesystem::path sourcepath = HyoutaUtils::IO::FilesystemPathFromUtf8(sourcePath);
    HyoutaUtils::IO::File f(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return std::format("Failed to open {}", sourcePath);
    }
    auto length = f.GetLength();
    if (!length) {
        return std::format("Failed to get length of {}", sourcePath);
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return std::format("Failed to allocate memory for {}", sourcePath);
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return std::format("Failed to read {}", sourcePath);
    }

    rapidjson::Document json;
    json.Parse<rapidjson::kParseFullPrecisionFlag | rapidjson::kParseNanAndInfFlag
                   | rapidjson::kParseCommentsFlag,
               rapidjson::UTF8<char>>(buffer.get(), *length);
    if (json.HasParseError()) {
        return std::format("Failed to parse JSON: {}",
                           rapidjson::GetParseError_En(json.GetParseError()));
    }
    if (!json.IsObject()) {
        return std::string("JSON root is not an object");
    }

    const auto root = json.GetObject();

    std::vector<SingleFileDirForPacking> fileinfos;
    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                SingleFileDirForPacking& fi = fileinfos.emplace_back();
                const auto fileobj = file.GetObject();
                const auto nameInArchive = JsonReadString(file, "NameInArchive");
                if (!nameInArchive) {
                    return std::string("JSON error: 'NameInArchive' missing or invalid");
                }
                const auto unknown1 = JsonReadUInt32(file, "Dir_Unknown1");
                if (!unknown1) {
                    return std::string("JSON error: 'Dir_Unknown1' missing or invalid");
                }
                const auto unknown3 = JsonReadUInt32(file, "Dir_Unknown3");
                if (!unknown3) {
                    return std::string("JSON error: 'Dir_Unknown3' missing or invalid");
                }
                const auto unknown5 = JsonReadUInt32(file, "Dir_Unknown5");
                if (!unknown5) {
                    return std::string("JSON error: 'Dir_Unknown5' missing or invalid");
                }
                const auto pathOnDisk = JsonReadString(file, "PathOnDisk");
                const auto compressedSize = JsonReadUInt32(file, "Dir_CompressedSize");
                const auto compressedSizeCopy = JsonReadUInt32(file, "Dir_CompressedSizeCopy");
                const auto isCompressed = JsonReadBool(file, "Compressed");

                // sanity checks
                auto sjis = HyoutaUtils::TextUtils::Utf8ToShiftJis(nameInArchive->data(),
                                                                   nameInArchive->size());
                if (!sjis) {
                    return std::format("JSON error: 'NameInArchive' not Shift-JIS compatible ({})",
                                       *nameInArchive);
                }
                if (sjis->size() > fi.Metadata.Filename.size()) {
                    return std::format("JSON error: 'NameInArchive' too long ({})", *nameInArchive);
                }

                // fill in fileinfo
                fi.Metadata.Unknown1 = *unknown1;
                fi.Metadata.CompressedSize = 0;
                fi.Metadata.Unknown3 = *unknown3;
                fi.Metadata.CompressedSizeCopy = 0;
                fi.Metadata.Unknown5 = *unknown5;
                fi.Metadata.OffsetInDat = 0;
                fi.Metadata.Filename.fill('\0');
                for (size_t i = 0; i < sjis->size(); ++i) {
                    fi.Metadata.Filename[i] = (*sjis)[i];
                }

                if (pathOnDisk) {
                    fi.Path = sourcepath.parent_path().append(std::u8string_view(
                        (const char8_t*)pathOnDisk->data(),
                        ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size()));
                }
                fi.ShouldCompress = isCompressed ? *isCompressed : true;
                fi.ShouldOverrideCompressedSizeCopy = false;
                if (compressedSize && compressedSizeCopy
                    && *compressedSize != *compressedSizeCopy) {
                    fi.ShouldOverrideCompressedSizeCopy = true;
                    fi.Metadata.CompressedSizeCopy = *compressedSizeCopy;
                }
            } else {
                return std::string("JSON error: File is not a JSON object");
            }
        }
    } else {
        return std::string("JSON error: 'Files' not found or not an array");
    }

    // count the number of 'dummy' entries at the end of the list
    size_t numberOfTrailingDummyEntries = 0;
    for (size_t i = fileinfos.size(); i > 0; --i) {
        if (fileinfos[i - 1].Path.has_value()) {
            break;
        }
        ++numberOfTrailingDummyEntries;
    }
    const size_t numberOfNormalEntries = fileinfos.size() - numberOfTrailingDummyEntries;

    HyoutaUtils::IO::File outfileDir;
    if (!outfileDir.OpenWithTempFilename(targetPathDir, HyoutaUtils::IO::OpenMode::Write)) {
        return std::string("Failed to open output .dir file");
    }
    auto outfileDirScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfileDir.Delete(); });

    HyoutaUtils::IO::File outfileDat;
    if (!outfileDat.OpenWithTempFilename(targetPathDat, HyoutaUtils::IO::OpenMode::Write)) {
        return std::string("Failed to open output .dat file");
    }
    auto outfileDatScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfileDat.Delete(); });

    std::array<char, 0x10> dirHeader{};
    std::array<char, 0x10> datHeader{};
    dirHeader[0] = 0x4c;
    dirHeader[1] = 0x42;
    dirHeader[2] = 0x20;
    dirHeader[3] = 0x44;
    dirHeader[4] = 0x49;
    dirHeader[5] = 0x52;
    dirHeader[6] = 0x1a;
    dirHeader[7] = 0x00;
    datHeader[0] = 0x4c;
    datHeader[1] = 0x42;
    datHeader[2] = 0x20;
    datHeader[3] = 0x44;
    datHeader[4] = 0x41;
    datHeader[5] = 0x54;
    datHeader[6] = 0x1a;
    datHeader[7] = 0x00;
    WriteUInt32(&dirHeader[8], ToEndian(static_cast<uint32_t>(fileinfos.size()), endian));
    WriteUInt32(&datHeader[8], ToEndian(static_cast<uint32_t>(fileinfos.size()), endian));

    if (outfileDir.Write(dirHeader.data(), dirHeader.size()) != dirHeader.size()) {
        return std::string("Failed to write to output .dir file");
    }
    if (outfileDat.Write(datHeader.data(), datHeader.size()) != datHeader.size()) {
        return std::string("Failed to write to output .dat file");
    }

    // reserve space for the dat offsets, needs one more offset than files we have
    std::vector<char> datOffsets;
    datOffsets.resize((fileinfos.size() + 1) * 4);
    if (outfileDat.Write(datOffsets.data(), datOffsets.size()) != datOffsets.size()) {
        return std::string("Failed to write to output .dat file");
    }

    // now actually write the files and the dir header
    uint32_t offsetInDat = datHeader.size() + datOffsets.size();
    for (size_t i = 0; i < fileinfos.size(); ++i) {
        SingleFileDirForPacking& fi = fileinfos[i];
        if (i >= numberOfNormalEntries) {
            fi.Metadata.CompressedSize = 0;
            if (!fi.ShouldOverrideCompressedSizeCopy) {
                fi.Metadata.CompressedSizeCopy = 0;
            }
            fi.Metadata.OffsetInDat = 0;
        } else if (fi.Path.has_value()) {
            HyoutaUtils::IO::File infile(*fi.Path, HyoutaUtils::IO::OpenMode::Read);
            if (!infile.IsOpen()) {
                return std::format("Failed opening {}", (char*)fi.Path->u8string().c_str());
            }
            const auto uncompressedLength = infile.GetLength();
            if (!uncompressedLength) {
                return std::format("Failed getting size of {}", (char*)fi.Path->u8string().c_str());
            }
            auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
            if (infile.Read(uncompressedData.get(), *uncompressedLength) != *uncompressedLength) {
                return std::format("Failed to read {}", (char*)fi.Path->u8string().c_str());
            }
            infile.Close();

            uint32_t compressedSize = 0;
            if (fi.ShouldCompress) {
                size_t bound = CompressedFileBound(*uncompressedLength, maxChunkSize);
                auto compressedData = std::make_unique<char[]>(bound);
                const std::optional<uint32_t> compressResult = CompressFile(uncompressedData.get(),
                                                                            *uncompressedLength,
                                                                            compressedData.get(),
                                                                            bound,
                                                                            maxChunkSize,
                                                                            skipBitCompression,
                                                                            skipByteCompression,
                                                                            endian);
                if (!compressResult) {
                    return std::format("Failed to compress {}", (char*)fi.Path->u8string().c_str());
                }
                compressedSize = *compressResult;
                if (outfileDat.Write(compressedData.get(), compressedSize) != compressedSize) {
                    return std::string("Failed to write to output .dat file");
                }
            } else {
                if (*uncompressedLength > UINT32_MAX) {
                    return std::format("{} too big to put into .dir/.dat",
                                       (char*)fi.Path->u8string().c_str());
                }
                compressedSize = static_cast<uint32_t>(*uncompressedLength);
                if (outfileDat.Write(uncompressedData.get(), *uncompressedLength)
                    != *uncompressedLength) {
                    return std::string("Failed to write to output .dat file");
                }
            }
            fi.Metadata.CompressedSize = compressedSize;
            if (!fi.ShouldOverrideCompressedSizeCopy) {
                fi.Metadata.CompressedSizeCopy = compressedSize;
            }
            fi.Metadata.OffsetInDat = offsetInDat;
            WriteUInt32(datOffsets.data() + (i * 4), ToEndian(offsetInDat, endian));
            offsetInDat += compressedSize;
            WriteUInt32(datOffsets.data() + ((i + 1) * 4), ToEndian(offsetInDat, endian));
        } else {
            fi.Metadata.CompressedSize = 0;
            if (!fi.ShouldOverrideCompressedSizeCopy) {
                fi.Metadata.CompressedSizeCopy = 0;
            }
            fi.Metadata.OffsetInDat = offsetInDat;
            WriteUInt32(datOffsets.data() + (i * 4), ToEndian(offsetInDat, endian));
            WriteUInt32(datOffsets.data() + ((i + 1) * 4), ToEndian(offsetInDat, endian));
        }

        if (outfileDir.Write(&fi.Metadata, sizeof(fi.Metadata)) != sizeof(fi.Metadata)) {
            return std::string("Failed to write to output .dir file");
        }
    }

    if (!outfileDat.SetPosition(datHeader.size())) {
        return std::string("Failed to seek in output .dat file");
    }
    if (outfileDat.Write(datOffsets.data(), datOffsets.size()) != datOffsets.size()) {
        return std::string("Failed to write to output .dat file");
    }

    if (!outfileDir.Flush()) {
        return std::string("Failed to flush output .dir file");
    }
    if (!outfileDat.Flush()) {
        return std::string("Failed to flush output .dat file");
    }
    if (!outfileDir.Rename(targetPathDir)) {
        return std::string("Failed to rename output .dir file");
    }
    if (!outfileDat.Rename(targetPathDat)) {
        return std::string("Failed to rename output .dat file");
    }
    outfileDirScope.Dispose();
    outfileDatScope.Dispose();

    return RepackDirDatResult::Success;
}
} // namespace SenTools::DirDat
