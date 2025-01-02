#include "type1_compress.h"
#include "type1_compress_main.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "cpp-optparse/OptionParser.h"

#include "sen/pkg.h"
#include "sen/pkg_compress.h"
#include "util/file.h"
#include "util/memread.h"

namespace SenTools {
int Type1_Compress_Function(int argc, char** argv) {
    optparse::OptionParser parser;
    parser.description(Type1_Compress_ShortDescription);

    parser.usage("sentools " Type1_Compress_Name " [options] file.bin");
    parser.add_option("-o", "--output")
        .dest("output")
        .metavar("FILENAME")
        .help("The output file to compress to. Will be derived from input filename if not given.");

    const auto& options = parser.parse_args(argc, argv);
    const auto& args = parser.args();
    if (args.size() != 1) {
        parser.error(args.size() == 0 ? "No input file given." : "More than 1 input file given.");
        return -1;
    }

    std::string_view source(args[0]);
    std::string_view target;
    std::string tmp;
    if (auto* output_option = options.get("output")) {
        target = std::string_view(output_option->first_string());
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
                                 HyoutaUtils::EndianUtils::Endianness::LittleEndian)) {
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
