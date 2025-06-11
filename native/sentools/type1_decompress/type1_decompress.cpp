#include "type1_decompress.h"
#include "type1_decompress_main.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "sen/pkg_extract.h"
#include "util/args.h"
#include "util/file.h"
#include "util/memread.h"

namespace SenTools {
int Type1_Decompress_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "FILENAME",
        .Description =
            "The output file to decompress to. Will be derived from input filename if not given."};
    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_output}};
    static constexpr HyoutaUtils::Args args("sentools " Type1_Decompress_Name,
                                            "file.bin",
                                            Type1_Decompress_ShortDescription,
                                            args_array);
    auto parseResult = args.Parse(argc, argv);
    if (parseResult.IsError()) {
        printf("Argument error: %s\n\n\n", parseResult.GetErrorValue().c_str());
        args.PrintUsage();
        return -1;
    }

    const auto& options = parseResult.GetSuccessValue();
    if (options.FreeArguments.size() != 1) {
        printf("Argument error: %s\n\n\n",
               options.FreeArguments.size() == 0 ? "No input file given."
                                                 : "More than 1 input file given.");
        args.PrintUsage();
        return -1;
    }

    std::string_view source(options.FreeArguments[0]);
    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.TryGetString(&arg_output)) {
        target = std::string_view(*output_option);
    } else {
        tmp = std::string(source);
        tmp += ".dec";
        target = tmp;
    }

    auto result = DecompressType1(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<DecompressType1Result, std::string> DecompressType1(std::string_view source,
                                                                        std::string_view target) {
    HyoutaUtils::IO::File infile(std::string_view(source), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        return std::string("Failed to get size of input file.");
    }
    if (*filesize < 12) {
        return std::string("File too small to be type1 compressed.");
    }
    if (*filesize > 0xffff'ffffu) {
        return std::string("File too big to be type1 compressed.");
    }

    auto pkgMemory = std::make_unique_for_overwrite<char[]>(*filesize);
    if (!pkgMemory) {
        return std::string("Failed to allocate memory.");
    }
    if (infile.Read(pkgMemory.get(), *filesize) != *filesize) {
        return std::string("Failed read input file.");
    }

    const uint32_t uncompressedSize =
        HyoutaUtils::EndianUtils::FromEndian(HyoutaUtils::MemRead::ReadUInt32(pkgMemory.get()),
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian);

    auto buffer = std::make_unique<char[]>(uncompressedSize);
    if (!buffer) {
        return std::string("Failed to allocate memory.");
    }
    if (!SenLib::ExtractAndDecompressPkgFile(buffer.get(),
                                             uncompressedSize,
                                             pkgMemory.get(),
                                             static_cast<uint32_t>(*filesize),
                                             1,
                                             HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
        return std::string("Failed to decompress file.");
    }

    HyoutaUtils::IO::File outfile(std::string_view(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        return std::string("Failed to open output file.");
    }

    if (outfile.Write(buffer.get(), uncompressedSize) != uncompressedSize) {
        return std::string("Failed to write to output file.");
    }

    return DecompressType1Result::Success;
}
} // namespace SenTools
