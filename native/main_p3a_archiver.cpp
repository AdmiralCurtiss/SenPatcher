#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include "p3a/pack.h"
#include "p3a/packfs.h"
#include "p3a/packjson.h"
#include "p3a/structs.h"
#include "p3a/unpackfs.h"

static void PrintUsage() {
    printf(
        "Usage for extracting an archive:\n"
        "  p3a extract (path to p3a file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n"
        "Usage for packing an archive (simple):\n"
        "  p3a pack [options] (path to directory to pack) (path to new archive)\n"
        "  options are:\n"
        "    --compression none/lz4/zstd\n"
        "Any existing file at the new archive location will be overwritten!\n"
        "\n"
        "Usage for packing an archive (advanced):\n"
        "  p3a packjson (path to json) (path to new archive)\n"
        "The json file should describe all files to be packed.\n"
        "For a reference, pack an archive with the simple variant, then extract it.\n"
        "A json file that can be used to re-pack the archive will be generated in\n"
        "the output directory.\n"
        "\n"
        "\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage();
        return -1;
    }

    if (strcmp("extract", argv[1]) == 0) {
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

        if (!SenPatcher::UnpackP3A(std::filesystem::path(source.begin(), source.end()),
                                   std::filesystem::path(target.begin(), target.end()))) {
            printf("Unpacking failed.\n");
            return -1;
        }
        return 0;
    } else if (strcmp("pack", argv[1]) == 0) {
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
                    PrintUsage();
                    return -1;
                }
                ++idx;
                continue;
            }

            break;
        }

        if (argc - 2 < idx) {
            PrintUsage();
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
    } else if (strcmp("packjson", argv[1]) == 0) {
        if (argc < 4) {
            PrintUsage();
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

    PrintUsage();
    return -1;
}
