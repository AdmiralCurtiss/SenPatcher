#include <array>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "file.h"
#include "p3a/pack.h"
#include "p3a/packfs.h"
#include "p3a/packjson.h"
#include "p3a/structs.h"
#include "p3a/unpackfs.h"
#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "sen/pkg_extract.h"
#include "util/stream.h"
#include "util/text.h"

#include "senpatcher_version.h"

using CliToolFunctionT = int (*)(int argc, char** argv);
struct CliTool {
    const char* Name;
    const char* ShortDescription;
    CliToolFunctionT Function;
};

#define P3A_Extract_Name "P3A.Extract"
#define P3A_Extract_ShortDescription "Extract a *.p3a archive to a directory."
static int P3A_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(P3A_Extract_ShortDescription);

    parser.usage("sentools " P3A_Extract_Name " [options] archive.p3a");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

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


    if (!SenPatcher::UnpackP3A(std::filesystem::path(source.begin(), source.end()),
                               std::filesystem::path(target.begin(), target.end()))) {
        printf("Unpacking failed.\n");
        return -1;
    }
    return 0;
}

#define P3A_Pack_Name "P3A.Pack"
#define P3A_Pack_ShortDescription "Pack a *.p3a archive from the contents of a directory."
static int P3A_Pack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(P3A_Pack_ShortDescription);

    parser.usage("sentools " P3A_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");
    parser.add_option("-c", "--compression")
        .dest("compression")
        .metavar("TYPE")
        .help("Which compression to use for the files packed into the archive.")
        .choices({"none", "lz4", "zstd"})
        .set_default("none");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);

    SenPatcher::P3ACompressionType compressionType = SenPatcher::P3ACompressionType::None;
    if (options.is_set("compression")) {
        const auto& compressionString = options["compression"];
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::None;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::LZ4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            compressionType = SenPatcher::P3ACompressionType::ZSTD;
        } else {
            parser.error("Invalid compression type.");
            return -1;
        }
    }


    if (!SenPatcher::PackP3AFromDirectory(std::filesystem::path(source.begin(), source.end()),
                                          std::filesystem::path(target.begin(), target.end()),
                                          compressionType)) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

#define P3A_Repack_Name "P3A.Repack"
#define P3A_Repack_ShortDescription "Repack a previously extracted *.p3a archive."
static int P3A_Repack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        P3A_Repack_ShortDescription
        "\n\n"
        "This re-packages a previously extracted achive and keeps all the metadata "
        "as best as possible. For example, the file order will be preserved, and "
        "the same compression type will be used for each file.\n\n"
        "To use this, point the program at the __p3a.json that was generated during "
        "the archive extraction. You can also modify this file for some advanced packing features "
        "that are not available through the standard directory packing interface.");

    parser.usage("sentools " P3A_Repack_Name " [options] __p3a.json");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);


    if (!SenPatcher::PackP3AFromJsonFile(std::filesystem::path(source.begin(), source.end()),
                                         std::filesystem::path(target.begin(), target.end()))) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

#define PKG_Extract_Name "PKG.Extract"
#define PKG_Extract_ShortDescription "Extract a *.pkg archive to a directory."
static int PKG_Extract_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Extract_ShortDescription);

    parser.usage("sentools " PKG_Extract_Name " [options] archive.pkg");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to extract to. Will be derived from input filename if not "
            "given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

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
    SenPatcher::IO::File infile(sourcepath, SenPatcher::IO::OpenMode::Read);
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

    for (size_t i = 0; i < pkg.FileCount; ++i) {
        const auto& pkgFile = pkg.Files[i];

        std::unique_ptr<char[]> buffer;
        size_t length;
        if (!SenLib::ExtractAndDecompressPkgFile(
                buffer, length, pkgFile, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            printf("Failed to extract file %zu from pkg.\n", i);
            return -1;
        }

        const auto& pkgName = pkgFile.Filename;
        std::u8string_view pkgNameSv(reinterpret_cast<const char8_t*>(pkgName.data()),
                                     pkgName.size());
        SenPatcher::IO::File outfile(targetpath
                                         / pkgNameSv.substr(0, pkgNameSv.find_first_of(u8'\0')),
                                     SenPatcher::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(buffer.get(), length) != length) {
            printf("Failed to write to output file.\n");
            return -1;
        }
    }

    return 0;
}

#define PKA_Convert_Name "PKA.Convert"
#define PKA_Convert_ShortDescription \
    "Convert a *.pka archive to the individual *.pkg files stored within."
static int PKA_Convert_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(
        PKA_Convert_ShortDescription
        "\n\n"
        "Note that this will duplicate every file that is stored in more than one "
        "pkg into every single of those pkg files. The converted archives will likely be much "
        "bigger than the input pka, so make sure you have enough disk space available.");

    parser.usage("sentools " PKA_Convert_Name " [options] assets.pka");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("DIRECTORY")
        .help(
            "The output directory to convert to. Will be derived from input filename if not "
            "given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

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
    SenPatcher::IO::File infile(sourcepath, SenPatcher::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        printf("Failed to open input file.\n");
        return -1;
    }

    SenLib::PkaHeader pkaHeader;
    if (!SenLib::ReadPkaFromFile(pkaHeader, infile)) {
        printf("Failed to read pka header.\n");
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

    for (size_t i = 0; i < pkaHeader.PkgCount; ++i) {
        SenLib::PkgHeader pkg;
        std::unique_ptr<char[]> buffer;
        if (!SenLib::ConvertPkaToSinglePkg(pkg, buffer, pkaHeader, i, infile)) {
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
        }

        std::unique_ptr<char[]> ms;
        size_t msSize;
        if (!SenLib::CreatePkgInMemory(
                ms, msSize, pkg, HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
            printf("Failed to convert archive %zu to pkg.\n", i);
            return -1;
        }

        const auto& pkgName = pkaHeader.Pkgs[i].PkgName;
        std::u8string_view pkgNameSv(reinterpret_cast<const char8_t*>(pkgName.data()),
                                     pkgName.size());
        SenPatcher::IO::File outfile(targetpath
                                         / pkgNameSv.substr(0, pkgNameSv.find_first_of(u8'\0')),
                                     SenPatcher::IO::OpenMode::Write);
        if (!outfile.IsOpen()) {
            printf("Failed to open output file.\n");
            return -1;
        }

        if (outfile.Write(ms.get(), msSize) != msSize) {
            printf("Failed to write to output file.\n");
            return -1;
        }
    }

    return 0;
}

#define PKG_Pack_Name "PKG.Pack"
#define PKG_Pack_ShortDescription "Pack a *.pkg archive from the contents of a directory."
static int PKG_Pack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Pack_ShortDescription);

    parser.usage("sentools " PKG_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input directory given."
                                      : "More than 1 input directory given.");
        return -1;
    }

    if (!options.is_set("output")) {
        parser.error("No output filename given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target(options["output"]);


    std::vector<SenLib::PkgFile> fileinfos;
    std::vector<std::unique_ptr<char[]>> filedatas;
    {
        std::filesystem::path rootDir(source);
        std::error_code ec;
        std::filesystem::directory_iterator iterator(rootDir, ec);
        if (ec) {
            return false;
        }
        for (auto const& entry : iterator) {
            if (entry.is_directory()) {
                continue;
            }

            const auto relativePath = std::filesystem::relative(entry.path(), rootDir, ec);
            if (relativePath.empty()) {
                printf("Error while collecting files.\n");
                return -1;
            }
            const auto filename = relativePath.u8string();
            const char8_t* filenameC = filename.c_str();

            std::array<char, 0x40> fn{};
            for (size_t i = 0; i < fn.size(); ++i) {
                const char c = static_cast<char>(filenameC[i]);
                if (c == '\0') {
                    break;
                }
                fn[i] = c;
            }
            auto& fi = fileinfos.emplace_back(SenLib::PkgFile{.Filename = fn});
            auto& fd = filedatas.emplace_back();

            SenPatcher::IO::File infile(entry.path(), SenPatcher::IO::OpenMode::Read);
            if (!infile.IsOpen()) {
                printf("Failed opening file.\n");
                return -1;
            }
            const auto uncompressedLength = infile.GetLength();
            if (!uncompressedLength) {
                printf("Failed getting size of file.\n");
                return -1;
            }
            if (*uncompressedLength > UINT32_MAX) {
                printf("File too big to put into pkg.\n");
                return -1;
            }
            auto uncompressedData = std::make_unique_for_overwrite<char[]>(*uncompressedLength);
            if (infile.Read(uncompressedData.get(), *uncompressedLength) != *uncompressedLength) {
                printf("Failed to read file.\n");
                return -1;
            }

            if (!SenLib::CompressPkgFile(fd,
                                         fi,
                                         uncompressedData.get(),
                                         static_cast<uint32_t>(*uncompressedLength),
                                         0,
                                         HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
                printf("Failed adding file to pkg.\n");
                return -1;
            }
        }
    }

    std::unique_ptr<char[]> ms;
    size_t msSize;
    if (!SenLib::CreatePkgInMemory(ms,
                                   msSize,
                                   fileinfos.data(),
                                   static_cast<uint32_t>(fileinfos.size()),
                                   0,
                                   HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        printf("Failed to create pkg.\n");
        return -1;
    }

    SenPatcher::IO::File outfile(std::filesystem::path(target), SenPatcher::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        printf("Failed to open output file.\n");
        return -1;
    }

    if (outfile.Write(ms.get(), msSize) != msSize) {
        printf("Failed to write to output file.\n");
        return -1;
    }

    return 0;
}

static constexpr auto CliTools = {
    CliTool{.Name = P3A_Extract_Name,
            .ShortDescription = P3A_Extract_ShortDescription,
            .Function = P3A_Extract_Function},
    CliTool{.Name = P3A_Pack_Name,
            .ShortDescription = P3A_Pack_ShortDescription,
            .Function = P3A_Pack_Function},
    CliTool{.Name = P3A_Repack_Name,
            .ShortDescription = P3A_Repack_ShortDescription,
            .Function = P3A_Repack_Function},
    CliTool{.Name = PKG_Extract_Name,
            .ShortDescription = PKG_Extract_ShortDescription,
            .Function = PKG_Extract_Function},
    CliTool{.Name = PKG_Pack_Name,
            .ShortDescription = PKG_Pack_ShortDescription,
            .Function = PKG_Pack_Function},
    CliTool{.Name = PKA_Convert_Name,
            .ShortDescription = PKA_Convert_ShortDescription,
            .Function = PKA_Convert_Function},
};

static void PrintUsage() {
    printf("SenTools from SenPatcher " SENPATCHER_VERSION "\n");
    printf("Select one of the following tools via the first argument:\n");
    for (const auto& tool : CliTools) {
        printf(" %-15s %s\n", tool.Name, tool.ShortDescription);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return -1;
    }

    const std::string_view name = argv[1];
    for (const auto& tool : CliTools) {
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals(name, tool.Name)) {
            return tool.Function(argc - 1, argv + 1);
        }
    }

    PrintUsage();
    return -1;
}
