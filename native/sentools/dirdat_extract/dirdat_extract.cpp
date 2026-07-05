#include "dirdat_extract.h"
#include "dirdat_extract_main.h"

#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace SenTools {
int DirDat_Extract_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "DIRECTORY",
        .Description =
            "The output directory to extract to. Will be derived from input .dir filename if not "
            "given."};
    static constexpr HyoutaUtils::Arg arg_json{
        .Type = HyoutaUtils::ArgTypes::Flag,
        .ShortKey = "j",
        .LongKey = "json",
        .Description =
            "If set, a __dir.json will be generated that contains information about the files in "
            "the archive. This file can be used to repack the archive with the DirDat.Repack "
            "option while preserving metadata within the archive."};
    static constexpr HyoutaUtils::Arg arg_dat{
        .Type = HyoutaUtils::ArgTypes::String,
        .LongKey = "dat",
        .Argument = "DAT",
        .Description =
            "Path to the .dat file. Will be derived from input .dir filename if not given."};
    static constexpr std::array<const HyoutaUtils::Arg*, 3> args_array{
        {&arg_output, &arg_json, &arg_dat}};
    static constexpr HyoutaUtils::Args args("sentools " DirDat_Extract_Name,
                                            "archive.dir",
                                            DirDat_Extract_ShortDescription,
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


    const bool generateJson = options.IsFlagSet(&arg_json);
    std::string_view sourcePathDir(options.FreeArguments[0]);
    std::string_view sourcePathDat;
    std::string_view targetPath;

    std::string sourcePathDatStorage;
    if (auto* output_option = options.TryGetString(&arg_dat)) {
        sourcePathDat = std::string_view(*output_option);
    } else {
        const size_t sep = sourcePathDir.find_last_of(
#ifdef BUILD_FOR_WINDOWS
            "\\"
#endif
            "/.");
        if (sep != std::string_view::npos && sourcePathDir[sep] == '.') {
            sourcePathDatStorage = std::string(sourcePathDir.substr(0, sep)) + ".dat";
        } else {
            sourcePathDatStorage = std::string(sourcePathDir) + ".dat";
        }
        sourcePathDat = sourcePathDatStorage;
    }

    std::string targetPathStorage;
    if (auto* output_option = options.TryGetString(&arg_output)) {
        targetPath = std::string_view(*output_option);
    } else {
        targetPathStorage = std::string(sourcePathDir);
        targetPathStorage += ".ex";
        targetPath = targetPathStorage;
    }

    auto result = DirDat::ExtractDirDat(sourcePathDir, sourcePathDat, targetPath, generateJson);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}
} // namespace SenTools

namespace SenTools::DirDat {
namespace {
struct DecompressionStruct {
    char* DecompressedData;
    const char* CompressedData;
    size_t AvailableBytesInDecompressedData;
    uint32_t NumberOfCompressedBytes;
};
} // namespace

#define CHECK_RANGE_COMPRESSED(n)                                     \
    do {                                                              \
        if ((n) > numberOfCompressedBytes                             \
            || compressedBytesRead > numberOfCompressedBytes - (n)) { \
            return false;                                             \
        }                                                             \
    } while (false)
#define CHECK_RANGE_DECOMPRESSED(n)                                                 \
    do {                                                                            \
        if ((n) > availableBytesInDecompressedData                                  \
            || decompressedBytesWritten > availableBytesInDecompressedData - (n)) { \
            return false;                                                           \
        }                                                                           \
    } while (false)
#define CHECK_RANGE_BACKREF(n)                \
    do {                                      \
        if ((n) > decompressedBytesWritten) { \
            return false;                     \
        }                                     \
    } while (false)

bool DecompressChunk0(DecompressionStruct* decomp) {
    const size_t availableBytesInDecompressedData = decomp->AvailableBytesInDecompressedData;
    const uint32_t numberOfCompressedBytes = decomp->NumberOfCompressedBytes;
    uint32_t compressedBytesRead = 0;
    size_t decompressedBytesWritten = 0;
    const char* compressedData = decomp->CompressedData;
    char* decompressedData = decomp->DecompressedData;

    CHECK_RANGE_COMPRESSED(2);
    ++compressedData; // first byte is always 0, just skip it
    ++compressedBytesRead;

    uint8_t bit = 0;
    int bufferedBitCounter = 8;
    uint32_t bufferedBits = static_cast<uint8_t>(*compressedData);
    ++compressedData;
    ++compressedBytesRead;
#define GET_BIT(target)                                                      \
    do {                                                                     \
        if (bufferedBitCounter == 0) {                                       \
            CHECK_RANGE_COMPRESSED(2);                                       \
            const uint32_t lowBits = static_cast<uint8_t>(*compressedData);  \
            ++compressedData;                                                \
            ++compressedBytesRead;                                           \
            const uint32_t highBits = static_cast<uint8_t>(*compressedData); \
            ++compressedData;                                                \
            ++compressedBytesRead;                                           \
            bufferedBits = (highBits << 8) | lowBits;                        \
            bufferedBitCounter = 16;                                         \
        }                                                                    \
        --bufferedBitCounter;                                                \
        target = (bufferedBits & 1);                                         \
        bufferedBits >>= 1;                                                  \
    } while (false)
#define GET_N_BITS(target, n)       \
    target = 0;                     \
    for (int i = 0; i < (n); ++i) { \
        uint8_t b = 0;              \
        GET_BIT(b);                 \
        target = (target << 1) | b; \
    }

    while (true) {
        uint32_t offsetBehind;
        while (true) {
            GET_BIT(bit);
            if (bit == 0) {
                // directly copy a byte from the compressed data
                CHECK_RANGE_COMPRESSED(1);
                CHECK_RANGE_DECOMPRESSED(1);
                *decompressedData = *compressedData;
                ++decompressedData;
                ++decompressedBytesWritten;
                ++compressedData;
                ++compressedBytesRead;
                continue;
            }

            GET_BIT(bit);
            if (bit == 0) {
                // backref with small offset
                CHECK_RANGE_COMPRESSED(1);
                offsetBehind = static_cast<uint8_t>(*compressedData);
                ++compressedData;
                ++compressedBytesRead;
                if (offsetBehind == 0) {
                    return false;
                }
                break;
            }
            {
                uint32_t tmp = 0;
                GET_N_BITS(tmp, 5);
                CHECK_RANGE_COMPRESSED(1);
                offsetBehind = (tmp << 8) + static_cast<uint8_t>(*compressedData);
                ++compressedData;
                ++compressedBytesRead;
            }
            if (offsetBehind > 1) {
                // backref with large offset
                break;
            }
            if (offsetBehind == 0) {
                // end of compressed data
                assert((decomp->CompressedData + compressedBytesRead) == compressedData);
                assert((decomp->DecompressedData + decompressedBytesWritten) == decompressedData);
                decomp->CompressedData = compressedData;
                decomp->DecompressedData = decompressedData;
                decomp->AvailableBytesInDecompressedData -= decompressedBytesWritten;
                return true;
            }

            // write 14 to 4109 copies of a single byte
            GET_BIT(bit);
            uint32_t lengthRaw;
            if (bit == 0) {
                GET_N_BITS(lengthRaw, 4);
            } else {
                uint32_t tmp = 0;
                GET_N_BITS(tmp, 4);
                CHECK_RANGE_COMPRESSED(1);
                lengthRaw = (tmp << 8) + static_cast<uint8_t>(*compressedData);
                ++compressedData;
                ++compressedBytesRead;
            }
            CHECK_RANGE_COMPRESSED(1);
            const char byteToWrite = *compressedData;
            ++compressedData;
            ++compressedBytesRead;
            uint32_t length = (lengthRaw + 14);
            CHECK_RANGE_DECOMPRESSED(length);
            while (length > 0) {
                *decompressedData = byteToWrite;
                ++decompressedData;
                ++decompressedBytesWritten;
                --length;
            }
        }

        // backref, copy a run of bytes from the already decompressed data
        CHECK_RANGE_BACKREF(offsetBehind);
        const char* copyFrom = decompressedData - offsetBehind;
        uint32_t length;
        GET_BIT(bit);
        if (bit != 0) {
            length = 2;
        } else {
            GET_BIT(bit);
            if (bit != 0) {
                length = 3;
            } else {
                GET_BIT(bit);
                if (bit != 0) {
                    length = 4;
                } else {
                    GET_BIT(bit);
                    if (bit != 0) {
                        length = 5;
                    } else {
                        GET_BIT(bit);
                        if (bit == 0) {
                            CHECK_RANGE_COMPRESSED(1);
                            length = static_cast<uint8_t>(*compressedData) + 14;
                            ++compressedData;
                            ++compressedBytesRead;
                        } else {
                            uint32_t tmp3 = 0;
                            GET_N_BITS(tmp3, 3);
                            length = tmp3 + 6;
                        }
                    }
                }
            }
        }
        CHECK_RANGE_DECOMPRESSED(length);
        while (length > 0) {
            *decompressedData = *copyFrom;
            ++decompressedData;
            ++decompressedBytesWritten;
            ++copyFrom;
            --length;
        }
    }

#undef GET_BIT
#undef GET_N_BITS
}

bool DecompressChunk1(DecompressionStruct* decomp) {
    const size_t availableBytesInDecompressedData = decomp->AvailableBytesInDecompressedData;
    const uint32_t numberOfCompressedBytes = decomp->NumberOfCompressedBytes;
    uint32_t compressedBytesRead = 0;
    size_t decompressedBytesWritten = 0;
    const char* compressedData = decomp->CompressedData;
    char* decompressedData = decomp->DecompressedData;
    while (true) {
        if (numberOfCompressedBytes == compressedBytesRead) {
            break;
        }
        CHECK_RANGE_COMPRESSED(1);
        const uint8_t type = static_cast<uint8_t>(*compressedData);
        ++compressedData;
        ++compressedBytesRead;
        if ((type & 0x80) != 0) {
            // backref, copy a run of bytes from the already decompressed data
            CHECK_RANGE_COMPRESSED(1);
            const uint8_t offsetLowBits = static_cast<uint8_t>(*compressedData);
            ++compressedData;
            ++compressedBytesRead;
            const uint32_t offsetBehind = ((type & 0x1f) << 8) | offsetLowBits;
            if (offsetBehind == 0) {
                return false;
            }
            CHECK_RANGE_BACKREF(offsetBehind);
            const char* copyFrom = decompressedData - offsetBehind;
            int length = (((type & 0x60) >> 5) + 4);
            if (numberOfCompressedBytes != compressedBytesRead) {
                uint8_t nextByte = static_cast<uint8_t>(*compressedData);
                while ((nextByte & 0xe0) == 0x60) {
                    length = (length + (nextByte & 0x1f));
                    ++compressedData;
                    ++compressedBytesRead;
                    if (numberOfCompressedBytes == compressedBytesRead) {
                        // this is janky... the game only does the range check on the first byte
                        // read here, but due to the compressed data format this gets terminated by
                        // the 'more chunks?' marker as long as that one is < 0x60, which is pretty
                        // much always true as no file has that many chunks. so just pretend this is
                        // correctly checked for each byte, because in practice it works out...
                        break;
                    }
                    nextByte = static_cast<uint8_t>(*compressedData);
                }
            }
            CHECK_RANGE_DECOMPRESSED(length);
            while (length > 0) {
                *decompressedData = *copyFrom;
                ++decompressedData;
                ++decompressedBytesWritten;
                ++copyFrom;
                --length;
            }
        } else if ((type & 0x40) == 0) {
            // directly copy a run of bytes from the compressed data
            uint32_t length;
            if ((type & 0x20) == 0) {
                // 0 to 31 bytes
                length = static_cast<uint32_t>(type & 0x1f);
            } else {
                // 0 to 8191 bytes
                CHECK_RANGE_COMPRESSED(1);
                const uint8_t lengthLowBits = static_cast<uint8_t>(*compressedData);
                ++compressedData;
                ++compressedBytesRead;
                length = static_cast<uint32_t>(((type & 0x1f) << 8) + lengthLowBits);
            }
            CHECK_RANGE_COMPRESSED(length);
            CHECK_RANGE_DECOMPRESSED(length);
            while (length > 0) {
                *decompressedData = *compressedData;
                ++decompressedData;
                ++decompressedBytesWritten;
                ++compressedData;
                ++compressedBytesRead;
                --length;
            }
        } else {
            uint32_t length;
            char byteToWrite;
            if ((type & 0x10) == 0) {
                // write 4 to 19 copies of a single byte
                CHECK_RANGE_COMPRESSED(1);
                byteToWrite = *compressedData;
                ++compressedData;
                ++compressedBytesRead;
                length = static_cast<uint32_t>((type & 0xf) + 4);
            } else {
                // write 4 to 4099 copies of a single byte
                CHECK_RANGE_COMPRESSED(2);
                const uint8_t lengthLowBits = static_cast<uint8_t>(*compressedData);
                ++compressedData;
                ++compressedBytesRead;
                length = static_cast<uint32_t>((((type & 0xf) << 8) | lengthLowBits) + 4);
                byteToWrite = *compressedData;
                ++compressedData;
                ++compressedBytesRead;
            }
            CHECK_RANGE_DECOMPRESSED(length);
            while (length > 0) {
                *decompressedData = byteToWrite;
                ++decompressedData;
                ++decompressedBytesWritten;
                --length;
            }
        }
    }

    assert((decomp->CompressedData + compressedBytesRead) == compressedData);
    assert((decomp->DecompressedData + decompressedBytesWritten) == decompressedData);
    decomp->CompressedData = compressedData;
    decomp->DecompressedData = decompressedData;
    decomp->AvailableBytesInDecompressedData -= decompressedBytesWritten;

    return true;
}

struct ChunkInfo {
    size_t DecompressedSize;
    uint32_t ChunkSize;
    uint8_t CompressionType;
    uint8_t NextChunkMarker;
};

std::optional<size_t> DecompressFile(char* outBuffer,
                                     size_t outBufferSize,
                                     const char* inBuffer,
                                     size_t inBufferSize,
                                     HyoutaUtils::EndianUtils::Endianness endian,
                                     std::vector<ChunkInfo>& outChunkInfos) {
    const char* p = inBuffer;
    size_t rest = inBufferSize;

    DecompressionStruct tmp;
    tmp.DecompressedData = outBuffer;
    tmp.AvailableBytesInDecompressedData = outBufferSize;

    // File is compressed in one or more chunks of data. Each chunk can be decompressed
    // independently.
    // A single chunk consists of:
    // - 2 bytes [length of chunk - 1]
    // - n bytes compressed data
    // - 1 byte 'more chunks?' marker
    // If the 'more chunks?' marker is zero we finished decompressing the file, otherwise another
    // chunk follows. In the game files the byte always indicates the amount of remaining chunks,
    // but the game code only checks whether it's zero or not.
    // The compressed data is one of two formats, indicated by the first byte of the compressed
    // data. If it's zero it's using the bitstream-style compression algorithm (see
    // DecompressChunk0 for details), otherwise it's using the bytestream-style compression
    // algorithm (see DecompressChunk1 for details). Note that despite selecting the algorithm the
    // byte itself is also part of the compressed data.

    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    while (true) {
        if (rest < 3) {
            return std::nullopt;
        }
        const uint16_t chunkSize = FromEndian(ReadUInt16(p), endian);
        if (chunkSize == 0) {
            // the game does some nonsense in this case. it reads uint32 max bytes but considers the
            // compressed stream to be 0 bytes long, so just abort if this happens.
            return std::nullopt;
        }
        const uint32_t actualChunkSize = (chunkSize + 1);
        if (actualChunkSize < 3 || actualChunkSize > rest) {
            return std::nullopt;
        }
        tmp.CompressedData = (p + 2);
        tmp.NumberOfCompressedBytes = (actualChunkSize - 3);
        const size_t previousAvailableBytes = tmp.AvailableBytesInDecompressedData;
        const bool isChunk0Type = (p[2] == 0);
        const bool success = isChunk0Type ? DecompressChunk0(&tmp) : DecompressChunk1(&tmp);
        if (!success) {
            return std::nullopt;
        }
        const uint8_t moreChunksMarker = static_cast<uint8_t>(p[actualChunkSize - 1]);
        outChunkInfos.emplace_back(ChunkInfo{
            .DecompressedSize = previousAvailableBytes - tmp.AvailableBytesInDecompressedData,
            .ChunkSize = actualChunkSize,
            .CompressionType = static_cast<uint8_t>(isChunk0Type ? 0 : 1),
            .NextChunkMarker = moreChunksMarker});
        if (moreChunksMarker == 0) {
            if (rest != actualChunkSize) {
                // did not consume all the compressed bytes, consider this a failure
                return std::nullopt;
            }
            return (outBufferSize - tmp.AvailableBytesInDecompressedData);
        }
        rest -= actualChunkSize;
        p += actualChunkSize;
    }
}

HyoutaUtils::Result<ExtractDirDatResult, std::string> ExtractDirDat(std::string_view sourcePathDir,
                                                                    std::string_view sourcePathDat,
                                                                    std::string_view targetPath,
                                                                    bool generateJson) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    static constexpr HyoutaUtils::EndianUtils::Endianness endian =
        HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    HyoutaUtils::IO::File infileDir(std::string_view(sourcePathDir),
                                    HyoutaUtils::IO::OpenMode::Read);
    if (!infileDir.IsOpen()) {
        return std::string("Failed to open input .dir file.");
    }
    HyoutaUtils::IO::File infileDat(std::string_view(sourcePathDat),
                                    HyoutaUtils::IO::OpenMode::Read);
    if (!infileDat.IsOpen()) {
        return std::string("Failed to open input .dat file.");
    }

    std::array<char, 0x10> dirHeader;
    if (infileDir.Read(dirHeader.data(), dirHeader.size()) != dirHeader.size()) {
        return std::string("Failed to read input .dir file header.");
    }
    std::array<char, 0x10> datHeader;
    if (infileDat.Read(datHeader.data(), datHeader.size()) != datHeader.size()) {
        return std::string("Failed to read input .dat file header.");
    }

    if (!(dirHeader[0] == 0x4c && dirHeader[1] == 0x42 && dirHeader[2] == 0x20
          && dirHeader[3] == 0x44 && dirHeader[4] == 0x49 && dirHeader[5] == 0x52
          && dirHeader[6] == 0x1a && dirHeader[7] == 0x00)) {
        return std::string("Invalid .dir file header.");
    }
    if (!(datHeader[0] == 0x4c && datHeader[1] == 0x42 && datHeader[2] == 0x20
          && datHeader[3] == 0x44 && datHeader[4] == 0x41 && datHeader[5] == 0x54
          && datHeader[6] == 0x1a && datHeader[7] == 0x00)) {
        return std::string("Invalid .dat file header.");
    }

    const uint32_t fileCountDir = FromEndian(ReadUInt32(&dirHeader[0x8]), endian);
    const uint32_t fileCountDat = FromEndian(ReadUInt32(&datHeader[0x8]), endian);
    if (fileCountDir != fileCountDat) {
        return std::string("Inconsistent file count between .dir and .dat files.");
    }

    const uint64_t entryCountDir = static_cast<uint64_t>(fileCountDir);
    const uint64_t entryCountDat = static_cast<uint64_t>(fileCountDir) + 1;
    const uint64_t entrySizeDir = entryCountDir * sizeof(SingleFileDir);
    const uint64_t entrySizeDat = entryCountDat * sizeof(uint32_t);
    auto dirEntries = std::make_unique_for_overwrite<SingleFileDir[]>(entryCountDir);
    auto datEntries = std::make_unique_for_overwrite<uint32_t[]>(entryCountDat);
    if (infileDir.Read(dirEntries.get(), entrySizeDir) != entrySizeDir) {
        return std::string("Failed to read .dir entries.");
    }
    if (infileDat.Read(datEntries.get(), entrySizeDat) != entrySizeDat) {
        return std::string("Failed to read .dat entries.");
    }

    for (size_t i = 0; i < entryCountDir; ++i) {
        const SingleFileDir dirEntry = dirEntries[i];
        const std::string_view filename = HyoutaUtils::TextUtils::StripToNull(dirEntry.Filename);
        if (filename == "/_______.___") {
            // this is used as the 'invalid entry' marker
            continue;
        }

        // the game itself uses the dirEntry.CompressedSize and dirEntry.OffsetInDat fields, and
        // everything else seems to be ignored. but check it anyway since it's a useful reference.
        if (dirEntry.CompressedSize != dirEntry.CompressedSizeCopy) {
            printf("Warning: Inconsistent compressed size in .dir entry for %.*s.\n",
                   static_cast<int>(filename.size()),
                   filename.data());
        }
        if (dirEntry.OffsetInDat != datEntries[i]) {
            printf("Warning: Inconsistent offset between .dir and .dat entries for %.*s.\n",
                   static_cast<int>(filename.size()),
                   filename.data());
        }
        if (datEntries[i] > datEntries[i + 1]) {
            printf("Warning: Invalid filesize in .dat entry for %.*s.\n",
                   static_cast<int>(filename.size()),
                   filename.data());
        }
        if (datEntries[i + 1] - datEntries[i] != dirEntry.CompressedSize) {
            printf("Warning: Inconsistent filesize between .dir and .dat entries for %.*s.\n",
                   static_cast<int>(filename.size()),
                   filename.data());
        }
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetPath, ec);
        if (ec) {
            return std::string("Failed to create output directory.");
        }
    }

    // this stupid format doesn't seem to store the filesize of the uncompressed file, so just
    // allocate a large buffer and hope everything fits...
    static constexpr size_t bufferSize = 50 * 1024 * 1024;
    auto buffer = std::make_unique_for_overwrite<char[]>(bufferSize);

    rapidjson::StringBuffer jsonbuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
    json.StartObject();

    json.Key("Files");
    json.StartArray();
    for (size_t i = 0; i < entryCountDir; ++i) {
        const SingleFileDir dirEntry = dirEntries[i];

        json.StartObject();
        json.Key("Dir_Unknown1");
        json.Uint(dirEntry.Unknown1);
        json.Key("Dir_CompressedSize");
        json.Uint(dirEntry.CompressedSize);
        json.Key("Dir_Unknown3");
        json.Uint(dirEntry.Unknown3);
        json.Key("Dir_CompressedSizeCopy");
        json.Uint(dirEntry.CompressedSizeCopy);
        json.Key("Dir_Unknown5");
        json.Uint(dirEntry.Unknown5);
        json.Key("Dir_OffsetInDat");
        json.Uint(dirEntry.OffsetInDat);
        json.Key("Dat_OffsetBegin");
        json.Uint(datEntries[i]);
        json.Key("Dat_OffsetEnd");
        json.Uint(datEntries[i + 1]);

        const std::string_view filename = HyoutaUtils::TextUtils::StripToNull(dirEntry.Filename);
        if (filename == "/_______.___") {
            json.Key("NameInArchive");
            json.String("/_______.___", sizeof("/_______.___") - 1, false);
            json.EndObject();
            continue;
        }

        auto filenameUtf8 =
            HyoutaUtils::TextUtils::ShiftJisToUtf8(filename.data(), filename.size());
        if (!filenameUtf8.has_value()) {
            return std::string("Invalid filename in archive (not Shift-JIS?).");
        }

        json.Key("NameInArchive");
        json.String(filenameUtf8->data(), filenameUtf8->size(), true);
        json.Key("PathOnDisk");
        json.String(filenameUtf8->data(), filenameUtf8->size(), true);

        std::string outpath = std::string(targetPath) + "/" + *filenameUtf8;
        HyoutaUtils::IO::File outfile(std::string_view(outpath), HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            return std::string("Failed to open output file.");
        }

        const uint32_t offset = dirEntry.OffsetInDat;
        const uint32_t length = dirEntry.CompressedSize;
        if (length == 0) {
            json.Key("Compressed");
            json.Bool(false);
            json.Key("UncompressedSize");
            json.Uint(0);
            json.EndObject();
            continue;
        }

        if (!infileDat.SetPosition(offset)) {
            return std::string("Failed to seek in .dat file.");
        }

        auto compressedMemory = std::make_unique_for_overwrite<char[]>(length);
        if (infileDat.Read(compressedMemory.get(), length) != length) {
            return std::string("Failed to read from .dat file.");
        }

        // not all files are compressed, but as far as I can tell there's absolutely nothing that
        // stores whether the file is compressed or not. the game itself just seems to know which
        // files are compressed and which aren't... so we just try to decompress, and if it works
        // great, and if it doesn't we assume uncompressed instead.
        std::vector<ChunkInfo> chunkInfos;
        const auto decompressionResult = DecompressFile(
            buffer.get(), bufferSize, compressedMemory.get(), length, endian, chunkInfos);
        if (decompressionResult) {
            json.Key("Compressed");
            json.Bool(true);
            json.Key("UncompressedSize");
            json.Uint(*decompressionResult);
            json.Key("Chunks");
            json.StartArray();
            for (const ChunkInfo& ci : chunkInfos) {
                json.StartObject();
                json.Key("ChunkSize");
                json.Uint(ci.ChunkSize);
                json.Key("CompressionType");
                json.Uint(ci.CompressionType);
                json.Key("NextChunkMarker");
                json.Uint(ci.NextChunkMarker);
                json.Key("UncompressedSize");
                json.Uint(ci.DecompressedSize);
                json.EndObject();
            }
            json.EndArray();
            if (outfile.Write(buffer.get(), *decompressionResult) != *decompressionResult) {
                return std::string("Failed to write decompressed file.");
            }
        } else {
            json.Key("Compressed");
            json.Bool(false);
            printf("Decompression of %s failed, assuming uncompressed.\n", filenameUtf8->c_str());
            if (outfile.Write(compressedMemory.get(), length) != length) {
                return std::string("Failed to write decompressed file.");
            }
        }

        json.EndObject();
    }
    json.EndArray();
    json.EndObject();

    if (generateJson) {
        std::string dirJsonPath(std::string(targetPath) + "/__dir.json");
        HyoutaUtils::IO::File f2(std::string_view(dirJsonPath), HyoutaUtils::IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return std::string("Failed to open __dir.json");
        }

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (f2.Write(jsonstring, jsonstringsize) != jsonstringsize) {
            return std::string("Failed to write __dir.json");
        }
    }

    return ExtractDirDatResult::Success;
}
} // namespace SenTools::DirDat
