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
    static constexpr std::array<const HyoutaUtils::Arg*, 2> args_array{
        {&arg_output_dir, &arg_output_dat}};
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

    auto result = DirDat::RepackDirDat(sourcePath, targetPathDir, targetPathDat);
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
static constexpr size_t DefaultUncompressedChunkSize = 0xfff0;
static constexpr size_t MaxChunkSizeForUncompressable = 0xfff0 - 3;
size_t CompressedFileBound(size_t uncompressedLength) {
    if (uncompressedLength == 0) {
        return 6;
    }
    size_t worstChunkCount = ((uncompressedLength + (MaxChunkSizeForUncompressable - 1))
                              / MaxChunkSizeForUncompressable);
    size_t bound = worstChunkCount * 0x10000;
    return bound;
}

#define CHECK_RANGE_COMPRESSED(n)                                                              \
    do {                                                                                       \
        if ((n) > compressedBufferLength || compressedOffset > compressedBufferLength - (n)) { \
            return std::nullopt;                                                               \
        }                                                                                      \
    } while (false)

// returns std::nullopt on failure. otherwise returns length of compressed data.
std::optional<uint32_t> CompressChunk0(const char* uncompressedData,
                                       size_t uncompressedChunkLength,
                                       char* compressedData,
                                       size_t compressedBufferLength) {
    return std::nullopt; // TODO
}

// returns std::nullopt on failure. otherwise returns length of compressed data.
std::optional<uint32_t> CompressChunk1(const char* uncompressedData,
                                       size_t uncompressedChunkLength,
                                       char* compressedData,
                                       size_t compressedBufferLength) {
    return std::nullopt; // TODO
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
            std::min<size_t>(DefaultUncompressedChunkSize, uncompressedRest);
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
        std::optional<uint32_t> compressedChunkSize0 =
            CompressChunk0(uncompressedData + uncompressedOffset,
                           uncompressedChunkLength,
                           tmpBuffer0.data(),
                           std::min(tmpBuffer0.size(), spaceForCompressedData));
        std::optional<uint32_t> compressedChunkSize1 =
            CompressChunk1(uncompressedData + uncompressedOffset,
                           uncompressedChunkLength,
                           tmpBuffer1.data(),
                           std::min(tmpBuffer1.size(), spaceForCompressedData));
        if (compressedChunkSize1.has_value()
            && (!compressedChunkSize0.has_value()
                || *compressedChunkSize1 <= *compressedChunkSize0)) {
            assert(*compressedChunkSize1 <= (0xffff - 2));
            compressedChunkLength = *compressedChunkSize1 + 2;
            std::memcpy(
                compressedData + compressedOffset + 2, tmpBuffer1.data(), *compressedChunkSize1);
        } else if (compressedChunkSize0.has_value()) {
            assert(*compressedChunkSize0 <= (0xffff - 2));
            compressedChunkLength = *compressedChunkSize0 + 2;
            std::memcpy(
                compressedData + compressedOffset + 2, tmpBuffer0.data(), *compressedChunkSize0);
        } else {
            uncompressedChunkLength =
                std::min<size_t>(MaxChunkSizeForUncompressable, uncompressedRest);
            std::optional<uint32_t> compressedChunkSize =
                WriteUncompressableChunk(uncompressedData + uncompressedOffset,
                                         uncompressedChunkLength,
                                         compressedData + (compressedOffset + 2),
                                         spaceForCompressedData);
            if (!compressedChunkSize.has_value() || *compressedChunkSize > (0xffff - 2)) {
                return std::nullopt;
            }
            compressedChunkLength = *compressedChunkSize + 2;
        }

        WriteUInt16(&compressedData[compressedOffset], ToEndian(compressedChunkLength, endian));
        compressedOffset += compressedChunkLength;
        uncompressedOffset += uncompressedChunkLength;
        uncompressedRest -= uncompressedChunkLength;
        bool moreChunks = (uncompressedRest > 0);
        compressedData[compressedOffset] = (moreChunks ? 1 : 0);
        ++compressedOffset;
    }

    return compressedOffset;
}

HyoutaUtils::Result<RepackDirDatResult, std::string> RepackDirDat(std::string_view sourcePath,
                                                                  std::string_view targetPathDir,
                                                                  std::string_view targetPathDat) {
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
                size_t bound = CompressedFileBound(*uncompressedLength);
                auto compressedData = std::make_unique<char[]>(bound);
                const std::optional<uint32_t> compressResult = CompressFile(uncompressedData.get(),
                                                                            *uncompressedLength,
                                                                            compressedData.get(),
                                                                            bound,
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
