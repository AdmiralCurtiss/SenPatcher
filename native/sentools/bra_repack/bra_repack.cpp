#include "bra_repack.h"

#include <array>
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

#include "zlib/zlib.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include "../bra_extract/bra_extract.h"
#include "bra_repack_main.h"
#include "util/align.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/memwrite.h"
#include "util/scope.h"
#include "util/text.h"


namespace SenTools {
static bool DeflateToFile(const char* buffer,
                          size_t length,
                          HyoutaUtils::IO::File& outfile,
                          int level = 9) {
    // adapted from https://zlib.net/zpipe.c which is public domain
    static constexpr size_t CHUNK = 16384;

    int ret;
    int flush;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, level, Z_DEFLATED, -15, 9, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return false;
    }

    /* compress until end of file */
    const char* next = buffer;
    size_t rest = length;
    do {
        uint32_t blockSize = rest > 0xffff'0000u ? 0xffff'0000u : static_cast<uint32_t>(rest);
        strm.avail_in = blockSize;
        flush = (blockSize == rest) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = const_cast<z_const Bytef*>(reinterpret_cast<const Bytef*>(next));

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);   /* no bad return value */
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (outfile.Write(out, have) != have) {
                (void)deflateEnd(&strm);
                return false;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */

        next += blockSize;
        rest -= blockSize;

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END); /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return true;
}

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

int BRA_Repack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{.Type = HyoutaUtils::ArgTypes::String,
                                                 .ShortKey = "o",
                                                 .LongKey = "output",
                                                 .Argument = "FILENAME",
                                                 .Description =
                                                     "The output filename. Must be given."};
    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_output}};
    static constexpr HyoutaUtils::Args args(
        "sentools " BRA_Repack_Name,
        "__bra.json",
        BRA_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, extract with the -j option, then point this program at the __bra.json that "
        "was generated during the archive extraction. You can also modify this file for some "
        "advanced packing features that are not available through the standard directory packing "
        "interface.",
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

    auto* output_option = options.TryGetString(&arg_output);
    if (output_option == nullptr) {
        printf("Argument error: %s\n\n\n", "No output filename given.");
        args.PrintUsage();
        return -1;
    }

    std::string_view source(options.FreeArguments[0]);
    std::string_view target(*output_option);

    auto result = RepackBra(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<RepackBraResult, std::string> RepackBra(std::string_view source,
                                                            std::string_view target) {
    using namespace HyoutaUtils::MemWrite;
    using HyoutaUtils::EndianUtils::ToEndian;
    static constexpr auto LE = HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    std::filesystem::path sourcepath = HyoutaUtils::IO::FilesystemPathFromUtf8(source);
    HyoutaUtils::IO::File f(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!f.IsOpen()) {
        return std::format("Failed to open {}", source);
    }
    auto length = f.GetLength();
    if (!length) {
        return std::format("Failed to get length of {}", source);
    }
    auto buffer = std::make_unique_for_overwrite<char[]>(*length);
    if (!buffer) {
        return std::format("Failed to allocate memory for {}", source);
    }
    if (f.Read(buffer.get(), *length) != *length) {
        return std::format("Failed to read {}", source);
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

    HyoutaUtils::IO::File outfile;
    if (!outfile.OpenWithTempFilename(target, HyoutaUtils::IO::OpenMode::Write)) {
        return std::string("Failed to open output file");
    }
    auto outfileScope = HyoutaUtils::MakeDisposableScopeGuard([&]() { outfile.Delete(); });

    // we fill this in at the end
    std::array<char, 0x10> header{};
    if (outfile.Write(header.data(), header.size()) != header.size()) {
        return std::string("Failed to write to output file");
    }

    std::vector<SenTools::BraFileInfo> fileinfos;
    const auto files = root.FindMember("Files");
    if (files != root.MemberEnd() && files->value.IsArray()) {
        for (const auto& file : files->value.GetArray()) {
            if (file.IsObject()) {
                const auto fileobj = file.GetObject();
                const auto nameInArchive = JsonReadString(file, "NameInArchive");
                if (!nameInArchive) {
                    return std::string("JSON error: 'NameInArchive' missing or invalid");
                }
                const auto pathOnDisk = JsonReadString(file, "PathOnDisk");
                if (!pathOnDisk) {
                    return std::string("JSON error: 'PathOnDisk' missing or invalid");
                }
                const auto compressionType = JsonReadUInt32(file, "CompressionType");
                if (!compressionType) {
                    return std::string("JSON error: 'CompressionType' missing or invalid");
                }
                const auto unknown1 = JsonReadUInt32(file, "Unknown1");
                if (!unknown1) {
                    return std::string("JSON error: 'Unknown1' missing or invalid");
                }
                const auto unknown2 = JsonReadUInt32(file, "Unknown2");
                if (!unknown2) {
                    return std::string("JSON error: 'Unknown2' missing or invalid");
                }
                const auto unknown3 = JsonReadUInt32(file, "Unknown3");
                if (!unknown3) {
                    return std::string("JSON error: 'Unknown3' missing or invalid");
                }
                const auto unknown4 = JsonReadUInt32(file, "Unknown4");
                if (!unknown4) {
                    return std::string("JSON error: 'Unknown4' missing or invalid");
                }
                const auto unknown5 = JsonReadUInt32(file, "Unknown5");
                if (!unknown5) {
                    return std::string("JSON error: 'Unknown5' missing or invalid");
                }

                // sanity checks
                auto sjis = HyoutaUtils::TextUtils::Utf8ToShiftJis(nameInArchive->data(),
                                                                   nameInArchive->size());
                if (!sjis) {
                    return std::format("JSON error: 'NameInArchive' not Shift-JIS compatible ({})",
                                       *nameInArchive);
                }
                if (sjis->size() >= 0x60) {
                    return std::format("JSON error: 'NameInArchive' too long ({})", *nameInArchive);
                }
                if (!(*compressionType == 0 || *compressionType == 6)) {
                    return std::string("JSON error: 'CompressionType' invalid");
                }

                auto& fi = fileinfos.emplace_back();

                HyoutaUtils::IO::File infile(
                    sourcepath.parent_path().append(std::u8string_view(
                        (const char8_t*)pathOnDisk->data(),
                        ((const char8_t*)pathOnDisk->data()) + pathOnDisk->size())),
                    HyoutaUtils::IO::OpenMode::Read);
                if (!infile.IsOpen()) {
                    return std::format("Failed opening {}", *pathOnDisk);
                }
                const auto uncompressedLength = infile.GetLength();
                if (!uncompressedLength) {
                    return std::format("Failed getting size of {}", *pathOnDisk);
                }
                if (*uncompressedLength > UINT32_MAX) {
                    return std::format("{} too big to put into bra", *pathOnDisk);
                }
                auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
                if (infile.Read(uncompressedData.get(), *uncompressedLength)
                    != *uncompressedLength) {
                    return std::format("Failed to read {}", *pathOnDisk);
                }

                // first a dummy header, we fill this in afterwards
                auto fileheaderPos = outfile.GetPosition();
                if (!fileheaderPos) {
                    return std::string("Failed to get position in output file");
                }
                std::array<char, 0x10> fileheader{};
                if (outfile.Write(fileheader.data(), fileheader.size()) != fileheader.size()) {
                    return std::string("Failed to write to output file");
                }

                if (*compressionType != 0) {
                    if (!DeflateToFile(uncompressedData.get(), *uncompressedLength, outfile, 9)) {
                        return std::string("Failed to compress");
                    }
                } else {
                    if (outfile.Write(uncompressedData.get(), *uncompressedLength)
                        != *uncompressedLength) {
                        return std::string("Failed to write to output file");
                    }
                }

                const auto fileEndPos = outfile.GetPosition();
                if (!fileEndPos) {
                    return std::string("Failed to get position in output file");
                }

                const auto compressedLengthIncludingHeader = (*fileEndPos - *fileheaderPos);
                if (compressedLengthIncludingHeader > UINT32_MAX) {
                    return std::format("{} too big to put into bra (after compression)",
                                       *pathOnDisk);
                }

                // fill in fileinfo
                fi.Unknown1 = *unknown1;
                fi.Unknown2 = *unknown2;
                fi.CompressedSize = static_cast<uint32_t>(compressedLengthIncludingHeader);
                fi.UncompressedSize = static_cast<uint32_t>(*uncompressedLength);
                fi.PathLength = HyoutaUtils::AlignUp(sjis->size(), 2);
                fi.Unknown3 = *unknown3;
                fi.DataPosition = *fileheaderPos;
                fi.FileHeader_UncompressedSize = fi.UncompressedSize;
                fi.FileHeader_CompressedSize = (fi.CompressedSize - 0x10);
                fi.FileHeader_Unknown2 = *unknown2;
                fi.FileHeader_CompressionType = *compressionType;
                fi.FileHeader_Unknown4 = *unknown4;
                fi.FileHeader_Unknown5 = *unknown5;
                fi.Path = std::move(*sjis);

                // update header
                if (!outfile.SetPosition(*fileheaderPos)) {
                    return std::string("Failed to set position in output file");
                }
                WriteUInt32(&fileheader[0x0], ToEndian(fi.FileHeader_UncompressedSize, LE));
                WriteUInt32(&fileheader[0x4], ToEndian(fi.FileHeader_CompressedSize, LE));
                WriteUInt32(&fileheader[0x8], ToEndian(fi.FileHeader_Unknown2, LE));
                WriteUInt8(&fileheader[0xc], fi.FileHeader_CompressionType);
                WriteUInt8(&fileheader[0xd], fi.FileHeader_Unknown4);
                WriteUInt16(&fileheader[0xe], ToEndian(fi.FileHeader_Unknown5, LE));
                if (outfile.Write(fileheader.data(), fileheader.size()) != fileheader.size()) {
                    return std::string("Failed to write to output file");
                }
                if (!outfile.SetPosition(*fileEndPos)) {
                    return std::string("Failed to set position in output file");
                }
            } else {
                return std::string("JSON error: File is not a JSON object");
            }
        }
    } else {
        return std::string("JSON error: 'Files' not found or not an array");
    }

    const auto footerPos = outfile.GetPosition();
    if (!footerPos) {
        return std::string("Failed to get position in output file");
    }

    // write file footers
    for (const SenTools::BraFileInfo& fi : fileinfos) {
        if (fi.PathLength > 0x60) {
            return std::string("Internal error");
        }

        std::array<char, 0x18 + 0x60> filefooter{};
        WriteUInt32(&filefooter[0x00], ToEndian(fi.Unknown1, LE));
        WriteUInt32(&filefooter[0x04], ToEndian(fi.Unknown2, LE));
        WriteUInt32(&filefooter[0x08], ToEndian(fi.CompressedSize, LE));
        WriteUInt32(&filefooter[0x0c], ToEndian(fi.UncompressedSize, LE));
        WriteUInt16(&filefooter[0x10], ToEndian(fi.PathLength, LE));
        WriteUInt16(&filefooter[0x12], ToEndian(fi.Unknown3, LE));
        WriteUInt32(&filefooter[0x14], ToEndian(fi.DataPosition, LE));
        HyoutaUtils::TextUtils::WriteToFixedLengthBuffer(
            filefooter.data() + 0x18, 0x60, fi.Path, true);
        size_t filefooterLength = 0x18 + fi.PathLength;
        if (outfile.Write(filefooter.data(), filefooterLength) != filefooterLength) {
            return std::string("Failed to write to output file");
        }
    }

    // update and write header
    if (!outfile.SetPosition(0)) {
        return std::string("Failed to set position in output file");
    }
    WriteUInt32(&header[0x0], ToEndian(static_cast<uint32_t>(0x414450), LE));
    WriteUInt32(&header[0x4], ToEndian(static_cast<uint32_t>(2), LE));
    WriteUInt32(&header[0x8], ToEndian(static_cast<uint32_t>(*footerPos), LE));
    WriteUInt32(&header[0xc], ToEndian(static_cast<uint32_t>(fileinfos.size()), LE));
    if (outfile.Write(header.data(), header.size()) != header.size()) {
        return std::string("Failed to write to output file");
    }
    if (!outfile.Rename(target)) {
        return std::string("Failed to rename output file");
    }
    outfileScope.Dispose();

    return RepackBraResult::Success;
}
} // namespace SenTools
