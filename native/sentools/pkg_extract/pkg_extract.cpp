#include "pkg_extract.h"
#include "pkg_extract_main.h"

#include <cstdio>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
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
    parser.add_option("--referenced-pka")
        .dest("referenced-pka")
        .action(optparse::ActionType::Append)
        .metavar("PKA")
        .help(
            "Referenced pka file that could contain file data, see the --as-pka-reference option "
            "in PKA.Extract for details. Option can be provided multiple times. This is a "
            "nonstandard feature that the vanilla game does not handle.");

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

    std::span<const std::string> referencedPkaPaths;
    if (auto* referenced_pka_option = options.get("referenced-pka")) {
        referencedPkaPaths = referenced_pka_option->strings();
    }

    auto result = ExtractPkg(source, target, referencedPkaPaths, generateJson);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<ExtractPkgResult, std::string>
    ExtractPkg(std::string_view source,
               std::string_view target,
               std::span<const std::string> referencedPkaPaths,
               bool generateJson) {
    std::filesystem::path targetpath = HyoutaUtils::IO::FilesystemPathFromUtf8(target);
    HyoutaUtils::IO::File infile(std::string_view(source), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        return std::string("Failed to get size of input file.");
    }

    auto pkgMemory = std::make_unique_for_overwrite<char[]>(*filesize);
    if (!pkgMemory) {
        return std::string("Failed to allocate memory.");
    }
    if (infile.Read(pkgMemory.get(), *filesize) != *filesize) {
        return std::string("Failed read input file.");
    }

    std::vector<SenLib::ReferencedPka> referencedPkas;
    {
        referencedPkas.reserve(referencedPkaPaths.size());
        for (const auto& referencedPkaPath : referencedPkaPaths) {
            auto& refPka = referencedPkas.emplace_back();
            refPka.PkaFile.Open(std::filesystem::path(referencedPkaPath),
                                HyoutaUtils::IO::OpenMode::Read);
            if (!refPka.PkaFile.IsOpen()) {
                return std::string("Error opening referenced pka.");
            }
            if (!SenLib::ReadPkaFromFile(refPka.PkaHeader, refPka.PkaFile)) {
                return std::string("Error reading referenced pka.");
            }
        }
    }

    SenLib::PkgHeader pkg;
    if (!SenLib::ReadPkgFromMemory(
            pkg, pkgMemory.get(), *filesize, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        return std::string("Failed to parse pkg header.");
    }

    {
        std::error_code ec;
        std::filesystem::create_directories(targetpath, ec);
        if (ec) {
            return std::string("Failed to create output directoy.");
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
        uint32_t uncompressedSize = pkgFile.UncompressedSize;
        uint32_t compressedSize = pkgFile.CompressedSize;
        uint32_t flags = pkgFile.Flags;
        const char* data = pkgFile.Data;

        std::unique_ptr<char[]> pkaDataBuffer;
        bool isPkaRef = false;
        if ((flags & 0x80) && compressedSize == 0x20) {
            // this is a PKA reference
            const SenLib::PkaHashToFileData* pkaFileData = nullptr;
            SenLib::ReferencedPka* pka = nullptr;
            for (auto& refPka : referencedPkas) {
                auto* f = SenLib::FindFileInPkaByHash(
                    refPka.PkaHeader.Files.get(), refPka.PkaHeader.FilesCount, data);
                if (f && f->UncompressedSize == uncompressedSize) {
                    pkaFileData = f;
                    pka = &refPka;
                    break;
                }
            }
            if (!pkaFileData) {
                return std::format("Failed to find data for file {} in referenced pka files.", i);
            }
            if (!pka->PkaFile.SetPosition(pkaFileData->Offset)) {
                return std::format("Failed to seek in referenced pka.");
            }
            pkaDataBuffer = std::make_unique<char[]>(pkaFileData->CompressedSize);
            if (pka->PkaFile.Read(pkaDataBuffer.get(), pkaFileData->CompressedSize)
                != pkaFileData->CompressedSize) {
                return std::format("Failed to read from referenced pka.");
            }

            isPkaRef = true;
            data = pkaDataBuffer.get();
            compressedSize = pkaFileData->CompressedSize;
            flags = pkaFileData->Flags;
        }

        auto buffer = std::make_unique<char[]>(uncompressedSize);
        if (!buffer) {
            return std::string("Failed to allocate memory.");
        }
        if (!SenLib::ExtractAndDecompressPkgFile(
                buffer.get(),
                uncompressedSize,
                data,
                compressedSize,
                flags,
                HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            return std::format("Failed to extract file {} from pkg.", i);
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
            return std::string("Failed to open output file.");
        }

        if (outfile.Write(buffer.get(), uncompressedSize) != uncompressedSize) {
            return std::string("Failed to write to output file.");
        }

        json.Key("NameInArchive");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("PathOnDisk");
        json.String(pkgFile.Filename.data(), pkgFilenameLength);
        json.Key("Flags");
        json.Uint(flags);
        if (isPkaRef) {
            json.Key("IsPkaReference");
            json.Bool(true);
        }
        json.EndObject();
    }
    json.EndArray();
    json.EndObject();

    if (generateJson) {
        HyoutaUtils::IO::File f2(targetpath / L"__pkg.json", HyoutaUtils::IO::OpenMode::Write);
        if (!f2.IsOpen()) {
            return std::string("Failed to open __pkg.json.");
        }

        const char* jsonstring = jsonbuffer.GetString();
        const size_t jsonstringsize = jsonbuffer.GetSize();
        if (f2.Write(jsonstring, jsonstringsize) != jsonstringsize) {
            return std::string("Failed to write __pkg.json.");
        }
    }

    return ExtractPkgResult::Success;
}
} // namespace SenTools
