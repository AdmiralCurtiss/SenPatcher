#include "pkg_pack_main.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "util/args.h"
#include "util/endian.h"
#include "util/file.h"
#include "util/text.h"

namespace SenTools {
int PKG_Pack_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{.Type = HyoutaUtils::ArgTypes::String,
                                                 .ShortKey = "o",
                                                 .LongKey = "output",
                                                 .Argument = "FILENAME",
                                                 .Description =
                                                     "The output filename. Must be given."};
    static constexpr HyoutaUtils::Arg arg_compression{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "c",
        .LongKey = "compression",
        .Argument = "TYPE",
        .Description =
            "Which compression to use for the files packed into the archive. Note that lz4 and "
            "zstd are not supported by all games!\n"
            "Options are: 'none', 'type1', 'lz4', 'zstd'."};
    static constexpr std::array<const HyoutaUtils::Arg*, 2> args_array{
        {&arg_output, &arg_compression}};
    static constexpr HyoutaUtils::Args args(
        "sentools " PKG_Pack_Name, "directory", PKG_Pack_ShortDescription, args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input directory given."
                                                 : "More than 1 input directory given.");
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

    uint32_t flags = 0;
    if (auto* compression_option = options.TryGetString(&arg_compression)) {
        const auto& compressionString = *compression_option;
        if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("none", compressionString)) {
            flags = 0;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("type1", compressionString)) {
            flags = 1;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("lz4", compressionString)) {
            flags = 4;
        } else if (HyoutaUtils::TextUtils::CaseInsensitiveEquals("zstd", compressionString)) {
            flags = 0x10;
        } else {
            printf("Argument error: %s\n\n\n", "Invalid compression type.");
            args.PrintUsage();
            return -1;
        }
    }


    std::vector<SenLib::PkgFile> fileinfos;
    std::vector<std::unique_ptr<char[]>> filedatas;
    {
        std::filesystem::path rootDir = HyoutaUtils::IO::FilesystemPathFromUtf8(source);
        std::error_code ec;
        std::filesystem::directory_iterator iterator(rootDir, ec);
        if (ec) {
            return -1;
        }
        while (iterator != std::filesystem::directory_iterator()) {
            if (!iterator->is_directory()) {
                const auto relativePath = std::filesystem::relative(iterator->path(), rootDir, ec);
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

                HyoutaUtils::IO::File infile(iterator->path(), HyoutaUtils::IO::OpenMode::Read);
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
                if (infile.Read(uncompressedData.get(), *uncompressedLength)
                    != *uncompressedLength) {
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
            iterator.increment(ec);
            if (ec) {
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

    HyoutaUtils::IO::File outfile(target, HyoutaUtils::IO::OpenMode::Write);
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
