#include "type1_compress.h"
#include "type1_compress_main.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "util/args.h"
#include "util/file.h"
#include "util/memread.h"

namespace SenTools {
int Type1_Compress_Function(int argc, char** argv) {
    static constexpr HyoutaUtils::Arg arg_output{
        .Type = HyoutaUtils::ArgTypes::String,
        .ShortKey = "o",
        .LongKey = "output",
        .Argument = "FILENAME",
        .Description =
            "The output file to compress to. Will be derived from input filename if not given."};
    static constexpr std::array<const HyoutaUtils::Arg*, 1> args_array{{&arg_output}};
    static constexpr HyoutaUtils::Args args(
        "sentools " Type1_Compress_Name, "file.bin", Type1_Compress_ShortDescription, args_array);
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
        tmp += ".type1";
        target = tmp;
    }

    auto result = CompressType1(source, target);
    if (result.IsError()) {
        printf("%s\n", result.GetErrorValue().c_str());
        return -1;
    }

    return 0;
}

HyoutaUtils::Result<CompressType1Result, std::string> CompressType1(std::string_view source,
                                                                    std::string_view target) {
    HyoutaUtils::IO::File infile(std::string_view(source), HyoutaUtils::IO::OpenMode::Read);
    if (!infile.IsOpen()) {
        return std::string("Failed to open input file.");
    }
    auto filesize = infile.GetLength();
    if (!filesize) {
        return std::string("Failed to get size of input file.");
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

    std::unique_ptr<char[]> compressedDataBuffer;
    SenLib::PkgFile pkgFile;
    if (!SenLib::CompressPkgFile(compressedDataBuffer,
                                 pkgFile,
                                 pkgMemory.get(),
                                 static_cast<uint32_t>(*filesize),
                                 1,
                                 HyoutaUtils::EndianUtils::Endianness::LittleEndian,
                                 true)) {
        return std::string("Compressing failed.");
    }

    HyoutaUtils::IO::File outfile(std::string_view(target), HyoutaUtils::IO::OpenMode::Write);
    if (!outfile.IsOpen()) {
        return std::string("Failed to open output file.");
    }

    if (outfile.Write(compressedDataBuffer.get(), pkgFile.CompressedSize)
        != pkgFile.CompressedSize) {
        return std::string("Failed to write to output file.");
    }

    return CompressType1Result::Success;
}
} // namespace SenTools
