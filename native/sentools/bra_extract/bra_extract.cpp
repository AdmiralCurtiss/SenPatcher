#include "bra_extract_main.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "zlib/zlib.h"

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "util/endian.h"
#include "util/file.h"
#include "util/memread.h"
#include "util/text.h"

namespace {
struct BraFileInfo {
    // from the BRA footer
    uint32_t Unknown1 = 0;
    uint32_t Unknown2 = 0;
    uint32_t CompressedSize = 0;
    uint32_t UncompressedSize = 0;
    uint16_t PathLength = 0; // seems to be 2-byte aligned
    uint16_t Unknown3 = 0;
    uint32_t DataPosition = 0;

    // from the first 0x10 bytes of the individual file
    uint32_t FileHeader_UncompressedSize = 0;
    uint32_t FileHeader_CompressedSize = 0;
    uint32_t FileHeader_Unknown2 = 0; // same as Unknown2 in the footer
    uint8_t FileHeader_CompressionType = 0;
    uint8_t FileHeader_Unknown4 = 0;
    uint16_t FileHeader_Unknown5 = 0;

    std::string Path;
};

static std::string_view StripToNull(std::string_view sv) {
    for (size_t i = 0; i < sv.size(); ++i) {
        if (sv[i] == '\0') {
            return sv.substr(0, i);
        }
    }
    return sv;
}

static bool InflateToFile(const char* buffer, size_t length, HyoutaUtils::IO::File& outfile) {
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
    if (ret != Z_OK)
        return false;

    /* decompress until deflate stream ends or end of file */
    size_t bytesLeft = length;
    const char* in = buffer;
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
                case Z_NEED_DICT: ret = Z_DATA_ERROR; [[fallthrough]];
                case Z_DATA_ERROR:
                case Z_MEM_ERROR: (void)inflateEnd(&strm); return false;
            }
            have = CHUNK - strm.avail_out;
            if (outfile.Write(out, have) != have) {
                (void)inflateEnd(&strm);
                return false;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? true : false;
}
} // namespace

namespace SenTools {
int BRA_Extract_Function(int argc, char** argv) {
    using namespace HyoutaUtils::MemRead;
    using HyoutaUtils::EndianUtils::FromEndian;
    static constexpr auto LE = HyoutaUtils::EndianUtils::Endianness::LittleEndian;

    optparse::OptionParser parser;
    parser.description(BRA_Extract_ShortDescription);

    parser.usage("sentools " BRA_Extract_Name " [options] archive.bra");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");
    parser.add_option("-j", "--json")
        .dest("json")
        .action(optparse::ActionType::StoreTrue)
        .help(
            "If set, a __bra.json will be generated that contains information about the files in "
            "the archive.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    const bool generateJson = options["json"].flag();
    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }


    std::filesystem::path sourcepath = HyoutaUtils::IO::FilesystemPathFromUtf8(source);
    std::filesystem::path targetpath = HyoutaUtils::IO::FilesystemPathFromUtf8(target);
    HyoutaUtils::IO::File infile(sourcepath, HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file.\n");
        return -1;
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        printf("Failed to get size of input file.\n");
        return -1;
    }

    std::array<char, 0x10> header;
    if (infile.Read(header.data(), header.size()) != header.size()) {
        printf("Failed to read header.\n");
        return -1;
    }

    // magic + version
    if (!(header[0] == 0x50 && header[1] == 0x44 && header[2] == 0x41 && header[3] == 0x00
          && header[4] == 0x02 && header[5] == 0x00 && header[6] == 0x00 && header[7] == 0x00)) {
        printf("Invalid header.\n");
        return -1;
    }

    const uint32_t footerPosition = FromEndian(ReadUInt32(&header[0x8]), LE);
    const uint32_t fileCount = FromEndian(ReadUInt32(&header[0xc]), LE);
    if (footerPosition >= *filesize) {
        printf("Invalid header.\n");
        return -1;
    }

    if (!infile.SetPosition(footerPosition)) {
        printf("Failed to seek.\n");
        return -1;
    }

    const uint64_t footerSize = (*filesize - footerPosition);
    auto footerMemory = std::make_unique_for_overwrite<char[]>(footerSize);
    if (!footerMemory) {
        printf("Failed to allocate memory.\n");
        return -1;
    }
    if (infile.Read(footerMemory.get(), footerSize) != footerSize) {
        printf("Failed read input file.\n");
        return -1;
    }

    auto fileInfos = std::make_unique<BraFileInfo[]>(fileCount);
    size_t realFileCount = 0;
    {
        uint64_t offset = 0;
        for (size_t i = 0; i < fileCount; ++i) {
            if (offset + 0x18 > footerSize) {
                break;
            }

            const uint32_t unknown1 = FromEndian(ReadUInt32(&footerMemory[offset]), LE);
            const uint32_t unknown2 = FromEndian(ReadUInt32(&footerMemory[offset + 0x4]), LE);
            const uint32_t compressedSize = FromEndian(ReadUInt32(&footerMemory[offset + 0x8]), LE);
            const uint32_t uncompressedSize =
                FromEndian(ReadUInt32(&footerMemory[offset + 0xc]), LE);
            const uint16_t pathLength = FromEndian(ReadUInt16(&footerMemory[offset + 0x10]), LE);
            const uint16_t unknown3 = FromEndian(ReadUInt16(&footerMemory[offset + 0x12]), LE);
            const uint32_t dataPosition = FromEndian(ReadUInt32(&footerMemory[offset + 0x14]), LE);
            offset += 0x18;

            if (offset + pathLength > footerSize) {
                break;
            }

            const std::string_view path(&footerMemory[offset], pathLength);
            offset += pathLength;

            // note: the game itself reserves a struct of 0x70 bytes for the file entries, and only
            // keeps Unknown2, CompressedSize, UncompressedSize, DataPosition, and the file path up
            // to the first null (which is blindly assumed to fit into 0x60 bytes...); see 0x40eeaa
            auto& fileInfo = fileInfos[i];
            fileInfo.Unknown1 = unknown1;
            fileInfo.Unknown2 = unknown2;
            fileInfo.CompressedSize = compressedSize;
            fileInfo.UncompressedSize = uncompressedSize;
            fileInfo.PathLength = pathLength;
            fileInfo.Unknown3 = unknown3;
            fileInfo.DataPosition = dataPosition;
            fileInfo.Path = path;

            ++realFileCount;
        }
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            printf("Failed to create output directoy.\n");
            return -1;
        }
    }

    rapidjson::StringBuffer jsonbuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json(jsonbuffer);
    json.StartObject();

    json.Key("Files");
    json.StartArray();
    for (size_t i = 0; i < realFileCount; ++i) {
        json.StartObject();

        auto& fileInfo = fileInfos[i];

        auto pathShiftJis = StripToNull(fileInfo.Path);
        auto pathUtf8 =
            HyoutaUtils::TextUtils::ShiftJisToUtf8(pathShiftJis.data(), pathShiftJis.size());
        if (!pathUtf8) {
            printf("Unknown encoding for path.\n");
            return -1;
        }
        json.Key("NameInArchive");
        json.String(pathUtf8->data(), pathUtf8->size(), true);
        for (char& c : *pathUtf8) {
            if (c == '\\') {
                c = '/';
            }
        }
        std::u8string_view pathUtf8Sv(reinterpret_cast<const char8_t*>(pathUtf8->data()),
                                      pathUtf8->size());
        std::filesystem::path p = targetpath / pathUtf8Sv;
        {
            std::error_code ec;
            std::filesystem::create_directories(p.parent_path(), ec);
            if (ec) {
                printf("Failed to create output directoy.\n");
                return -1;
            }
        }

        if (fileInfo.CompressedSize < 0x10) {
            printf("Invalid data.\n");
            return -1;
        }

        auto buffer = std::make_unique_for_overwrite<char[]>(fileInfo.CompressedSize);
        if (!buffer) {
            printf("Failed to allocate memory.\n");
            return -1;
        }
        if (!infile.SetPosition(fileInfo.DataPosition)) {
            printf("Failed to seek.\n");
            return -1;
        }
        if (infile.Read(buffer.get(), fileInfo.CompressedSize) != fileInfo.CompressedSize) {
            printf("Failed to read file.\n");
            return -1;
        }

        // there's a bunch of redundant information in the first 0x10 bytes of each file, not sure
        // which is actually used by the game though. i think the byte at 0xc is the compression
        // type? see 0x41190a, deflate decompression loop at 0x4119c7
        fileInfo.FileHeader_UncompressedSize = FromEndian(ReadUInt32(&buffer[0x0]), LE);
        fileInfo.FileHeader_CompressedSize = FromEndian(ReadUInt32(&buffer[0x4]), LE);
        fileInfo.FileHeader_Unknown2 = FromEndian(ReadUInt32(&buffer[0x8]), LE);
        fileInfo.FileHeader_CompressionType = ReadUInt8(&buffer[0xc]);
        fileInfo.FileHeader_Unknown4 = ReadUInt8(&buffer[0xd]);
        fileInfo.FileHeader_Unknown5 = FromEndian(ReadUInt16(&buffer[0xe]), LE);

        HyoutaUtils::IO::File outfile(p, HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (fileInfo.FileHeader_UncompressedSize != fileInfo.UncompressedSize) {
            printf(
                "WARNING: File '%s': Uncompressed filesize mismatch between BRA header and file "
                "header.\n",
                pathUtf8->c_str());
        }
        if (fileInfo.FileHeader_CompressedSize != (fileInfo.CompressedSize - 0x10)) {
            printf(
                "WARNING: File '%s': Compressed filesize mismatch between BRA header and file "
                "header.\n",
                pathUtf8->c_str());
        }

        if (fileInfo.FileHeader_CompressionType == 0) {
            if ((fileInfo.CompressedSize - 0x10) != fileInfo.UncompressedSize) {
                printf(
                    "WARNING: File '%s': Compressed/uncompressed size mismatch for uncompressed "
                    "file.\n",
                    pathUtf8->c_str());
            }

            if (outfile.Write(buffer.get() + 0x10, fileInfo.CompressedSize - 0x10)
                != (fileInfo.CompressedSize - 0x10)) {
                printf("Failed to write to output file.\n");
                return -1;
            }
        } else {
            if (!InflateToFile(buffer.get() + 0x10, fileInfo.CompressedSize - 0x10, outfile)) {
                printf("Failed to decompress to output file.\n");
                return -1;
            }

            if (outfile.GetLength() != fileInfo.UncompressedSize) {
                printf("WARNING: File '%s': Decompressed into unexpected filesize.\n",
                       pathUtf8->c_str());
            }
        }

        json.Key("PathOnDisk");
        json.String(pathUtf8->data(), pathUtf8->size(), true);
        json.Key("CompressionType");
        json.Uint(fileInfo.FileHeader_CompressionType);
        json.Key("Unknown1");
        json.Uint(fileInfo.Unknown1);
        json.Key("Unknown2");
        json.Uint(fileInfo.Unknown2);
        json.Key("Unknown3");
        json.Uint(fileInfo.Unknown3);
        json.Key("Unknown4");
        json.Uint(fileInfo.FileHeader_Unknown4);
        json.Key("Unknown5");
        json.Uint(fileInfo.FileHeader_Unknown5);
        json.EndObject();
    }
    json.EndArray();
    json.EndObject();

    if (generateJson) {
        HyoutaUtils::IO::File f2(targetpath / L"__bra.json", HyoutaUtils::IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return false;
        }

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (f2.Write(jsonstring, jsonstringsize) != jsonstringsize) {
            return false;
        }
    }

    return 0;
}
} // namespace SenTools
