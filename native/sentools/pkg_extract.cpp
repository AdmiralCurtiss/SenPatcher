#include "pkg_extract.h"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "sen/pkg.h"
#include "sen/pkg_extract.h"
#include "util/file.h"

namespace SenTools {
int PKG_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Extract_ShortDescription);

    parser.usage("sentools " PKG_Extract_Name " [options] archive.pkg");
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
            "If set, a __pkg.json will be generated that contains information about the files in "
            "the archive. This file can be used to repack the archive with the PKG.Repack option "
            "while preserving compression types and file order within the archive.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    const bool generateJson = options.is_set("json");
    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (options.is_set("output")) {
        target = std::string_view(options["output"]);
    } else {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    }


    std::filesystem::path sourcepath(source.begin(), source.end());
    std::filesystem::path targetpath(target.begin(), target.end());
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

    auto pkgMemory = std::make_unique_for_overwrite<char[]>(*filesize);
    if (!pkgMemory) {
        printf("Failed to allocate memory.\n");
        return -1;
    }
    if (infile.Read(pkgMemory.get(), *filesize) != *filesize) {
        printf("Failed read input file.\n");
        return -1;
    }

    SenLib::PkgHeader pkg;
    if (!SenLib::ReadPkgFromMemory(
            pkg, pkgMemory.get(), *filesize, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        printf("Failed to parse pkg header.\n");
        return -1;
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
    json.Key("Unknown");
    json.Uint(pkg.Unknown);

    json.Key("Files");
    json.StartArray();
    for (size_t i = 0; i < pkg.FileCount; ++i) {
        json.StartObject();

        const auto& pkgFile = pkg.Files[i];

        auto buffer = std::make_unique<char[]>(pkgFile.UncompressedSize);
        if (!buffer) {
            printf("Failed to allocate memory.\n");
            return false;
        }
        if (!SenLib::ExtractAndDecompressPkgFile(
                buffer.get(),
                pkgFile.UncompressedSize,
                pkgFile.Data,
                pkgFile.CompressedSize,
                pkgFile.Flags,
                HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            printf("Failed to extract file %zu from pkg.\n", i);
            return -1;
        }

        const auto& pkgName = pkgFile.Filename;
        std::u8string_view pkgNameSv(reinterpret_cast<const char8_t*>(pkgName.data()),
                                     pkgName.size());
        const size_t pkgFilenameFirstNull = pkgNameSv.find_first_of(u8'\0');
        const size_t pkgFilenameLength = (pkgFilenameFirstNull == std::u8string_view::npos)
                                             ? pkgName.size()
                                             : pkgFilenameFirstNull;
        HyoutaUtils::IO::File outfile(targetpath / pkgNameSv.substr(0, pkgFilenameLength),
                                      HyoutaUtils::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(buffer.get(), pkgFile.UncompressedSize) != pkgFile.UncompressedSize) {
            printf("Failed to write to output file.\n");
            return -1;
        }

        json.Key("NameInArchive");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("PathOnDisk");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("Flags");
        json.Uint(pkgFile.Flags);
        json.EndObject();
    }
    json.EndArray();
    json.EndObject();

    if (generateJson) {
        HyoutaUtils::IO::File f2(targetpath / L"__pkg.json", HyoutaUtils::IO::OpenMode::Write);
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
