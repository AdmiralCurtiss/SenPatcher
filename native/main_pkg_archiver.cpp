#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include "file.h"
#include "sen/pka.h"
#include "sen/pka_to_pkg.h"
#include "sen/pkg.h"
#include "sen/pkg_extract.h"
#include "util/stream.h"

static void PrintUsage() {
    printf(
        "Usage for extracting a PKG:\n"
        "  pkg pkgextract (path to pkg file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n"
        "Usage for extracting a PKA to individual PKGs:\n"
        "  pkg pkaconvert (path to pka file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n"
        "\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return -1;
    }

    if (strcmp("pkgextract", argv[1]) == 0) {
        if (argc < 3) {
            PrintUsage();
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
        if (!SenLib::ReadPkgFromMemory(pkg,
                                       pkgMemory.get(),
                                       *filesize,
                                       HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
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
    } else if (strcmp("pkaconvert", argv[1]) == 0) {
        if (argc < 3) {
            PrintUsage();
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

    PrintUsage();
    return -1;
}
