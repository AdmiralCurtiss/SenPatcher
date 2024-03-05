#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include "p3a/pack.h"
#include "p3a/packjson.h"
#include "p3a/structs.h"
#include "p3a/unpack.h"

static void PrintUsage() {
    printf(
        "Usage for extracting an archive:\n"
        "  p3a extract (path to p3a file) [path to directory to extract to]\n"
        "Output directory will be input file + '.ex' if not given.\n"
        "Any existing files in the output directory may be overwritten!\n"
        "\n"
        "Usage for packing an archive (simple):\n"
        "  p3a pack (path to directory to pack) (path to new archive)\n"
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

        std::u8string_view source((const char8_t*)argv[2]);
        std::u8string_view target;
        std::u8string tmp;
        if (argc < 4) {
            tmp = std::u8string(source);
            tmp += (const char8_t*)".ex";
            target = tmp;
        } else {
            target = std::u8string_view((const char8_t*)argv[3]);
        }

        if (!SenPatcher::UnpackP3A(std::filesystem::path(source.begin(), source.end()),
                                   std::filesystem::path(target.begin(), target.end()))) {
            return -1;
        }
        return 0;
    } else if (strcmp("pack", argv[1]) == 0) {
        if (argc < 4) {
            PrintUsage();
            return -1;
        }

        std::u8string_view source((const char8_t*)argv[2]);
        std::u8string_view target((const char8_t*)argv[3]);
        if (!SenPatcher::PackP3AFromDirectory(std::filesystem::path(source.begin(), source.end()),
                                              std::filesystem::path(target.begin(), target.end()),
                                              SenPatcher::P3ACompressionType::LZ4)) {
            return -1;
        }
        return 0;
    } else if (strcmp("packjson", argv[1]) == 0) {
        if (argc < 4) {
            PrintUsage();
            return -1;
        }

        std::u8string_view source((const char8_t*)argv[2]);
        std::u8string_view target((const char8_t*)argv[3]);
        if (!SenPatcher::PackP3AFromJsonFile(std::filesystem::path(source.begin(), source.end()),
                                             std::filesystem::path(target.begin(), target.end()))) {
            return -1;
        }
        return 0;
    }

    PrintUsage();
    return -1;
}
