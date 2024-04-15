#include <array>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

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

static constexpr const char* P3A_Extract_Name = "P3A.Extract";
static constexpr const char* P3A_Extract_ShortDescription =
    "Extract a *.p3a archive to a directory.";
static void P3A_Extract_PrintUsage() {
    printf(
        "Usage for extracting an archive:\n"
        "  p3a extract (path to p3a file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n");
}
static int P3A_Extract_Function(int argc, char** argv) {
    if (argc < 3) {
        P3A_Extract_PrintUsage();
        return -1;
    }

    std::string_view source(argv[2]);
    std::string_view target;
    std::string tmp;
    if (argc < 4) {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    } else {
        target = std::string_view(argv[3]);
    }

    if (!SenPatcher::UnpackP3A(std::filesystem::path(source.begin(), source.end()),
                               std::filesystem::path(target.begin(), target.end()))) {
        printf("Unpacking failed.\n");
        return -1;
    }
    return 0;
}

static constexpr const char* P3A_Pack_Name = "P3A.Pack";
static constexpr const char* P3A_Pack_ShortDescription =
    "Pack a *.p3a archive from the contents of a directory.";
static void P3A_Pack_PrintUsage() {
    printf(
        "Usage for packing an archive (simple):\n"
        "  p3a pack [options] (path to directory to pack) (path to new archive)\n"
        "  options are:\n"
        "    --compression none/lz4/zstd\n"
        "Any existing file at the new archive location will be overwritten!\n"
        "\n");
}
static int P3A_Pack_Function(int argc, char** argv) {
    SenPatcher::P3ACompressionType compressionType = SenPatcher::P3ACompressionType::None;
    int idx = 2;
    while (idx < argc) {
        if (strcmp("--compression", argv[idx]) == 0) {
            ++idx;
            if (idx < argc) {
                if (strcmp("none", argv[idx]) == 0) {
                    compressionType = SenPatcher::P3ACompressionType::None;
                } else if (strcmp("lz4", argv[idx]) == 0) {
                    compressionType = SenPatcher::P3ACompressionType::LZ4;
                } else if (strcmp("zstd", argv[idx]) == 0) {
                    compressionType = SenPatcher::P3ACompressionType::ZSTD;
                } else {
                    printf("Invalid compression type.\n");
                    return -1;
                }
            } else {
                P3A_Pack_PrintUsage();
                return -1;
            }
            ++idx;
            continue;
        }

        break;
    }

    if (argc - 2 < idx) {
        P3A_Pack_PrintUsage();
        return -1;
    }

    std::string_view source(argv[idx]);
    std::string_view target(argv[idx + 1]);
    if (!SenPatcher::PackP3AFromDirectory(std::filesystem::path(source.begin(), source.end()),
                                          std::filesystem::path(target.begin(), target.end()),
                                          compressionType)) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

static constexpr const char* P3A_PackJSON_Name = "P3A.PackJSON";
static constexpr const char* P3A_PackJSON_ShortDescription =
    "Pack a *.p3a archive from a *.json file describing its contents.";
static void P3A_PackJSON_PrintUsage() {
    printf(
        "Usage for packing an archive (advanced):\n"
        "  p3a packjson (path to json) (path to new archive)\n"
        "The json file should describe all files to be packed.\n"
        "For a reference, pack an archive with the simple variant, then extract it.\n"
        "A json file that can be used to re-pack the archive will be generated in\n"
        "the output directory.\n"
        "\n");
}
static int P3A_PackJSON_Function(int argc, char** argv) {
    if (argc < 4) {
        P3A_PackJSON_PrintUsage();
        return -1;
    }

    std::string_view source(argv[2]);
    std::string_view target(argv[3]);
    if (!SenPatcher::PackP3AFromJsonFile(std::filesystem::path(source.begin(), source.end()),
                                         std::filesystem::path(target.begin(), target.end()))) {
        printf("Packing failed.\n");
        return -1;
    }
    return 0;
}

static constexpr const char* PKG_Extract_Name = "PKG.Extract";
static constexpr const char* PKG_Extract_ShortDescription =
    "Extract a *.pkg archive to a directory.";
static void PKG_Extract_PrintUsage() {
    printf(
        "Usage for extracting a PKG:\n"
        "  pkg pkgextract (path to pkg file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n");
}
static int PKG_Extract_Function(int argc, char** argv) {
    if (argc < 3) {
        PKG_Extract_PrintUsage();
        return -1;
    }

    std::string_view source(argv[2]);
    std::string_view target;
    std::string tmp;
    if (argc < 4) {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    } else {
        target = std::string_view(argv[3]);
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

static constexpr const char* PKA_Convert_Name = "PKA.Convert";
static constexpr const char* PKA_Convert_ShortDescription =
    "Convert a *.pka archive to the individual *.pkg files stored within.";
static void PKA_Convert_PrintUsage() {
    printf(
        "Usage for extracting a PKA to individual PKGs:\n"
        "  pkg pkaconvert (path to pka file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n");
}
static int PKA_Convert_Function(int argc, char** argv) {
    if (argc < 3) {
        PKA_Convert_PrintUsage();
        return -1;
    }

    std::string_view source(argv[2]);
    std::string_view target;
    std::string tmp;
    if (argc < 4) {
        tmp = std::string(source);
        tmp += ".ex";
        target = tmp;
    } else {
        target = std::string_view(argv[3]);
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

static constexpr const char* PKG_Pack_Name = "PKG.Pack";
static constexpr const char* PKG_Pack_ShortDescription =
    "Pack a *.pkg archive from the contents of a directory.";
static void PKG_Pack_PrintUsage() {
    printf(
        "Usage for packing a PKG:\n"
        "  pkg pkgpack [options] (path to directory to pack) (path to new archive)\n"
        "Any existing file at the new archive location will be overwritten!\n"
        "\n");
}
static int PKG_Pack_Function(int argc, char** argv) {
    int idx = 2;
    if (argc - 2 < idx) {
        PKG_Pack_PrintUsage();
        return -1;
    }

    std::string_view source(argv[idx]);
    std::string_view target(argv[idx + 1]);

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
    CliTool{.Name = P3A_PackJSON_Name,
            .ShortDescription = P3A_PackJSON_ShortDescription,
            .Function = P3A_PackJSON_Function},
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
            return tool.Function(argc, argv);
        }
    }

    PrintUsage();
    return -1;
}
