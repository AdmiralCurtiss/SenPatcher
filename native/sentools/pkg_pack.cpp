#include "pkg_pack.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "cpp-optparse/OptionParser.h"

#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/text.h"

namespace SenTools {
int PKG_Pack_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(PKG_Pack_ShortDescription);

    parser.usage("sentools " PKG_Pack_Name " [options] directory");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output filename. Must be given.");
    parser.add_option("-c", "--compression")
        .dest("compression")
        .metavar("TYPE")
        .help(
            "Which compression to use for the files packed into the archive. Note that lz4 and "
            "zstd are not supported by all games!")
        .choices({"none", "type1", "lz4", "zstd"})
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

    uint32_t flags = 0;
    if (options.is_set("compression")) {
        const auto& compressionString = options["compression"];
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            flags = 0;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("type1", compressionString)) {
            flags = 1;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            flags = 4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            flags = 0x10;
        } else {
            parser.error("Invalid compression type.");
            return -1;
        }
    }


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

            HyoutaUtils::IO::File infile(entry.path(), HyoutaUtils::IO::OpenMode::Read);
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
                                         flags,
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

    HyoutaUtils::IO::File outfile(std::filesystem::path(target), HyoutaUtils::IO::OpenMode::Write);
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
} // namespace SenTools
